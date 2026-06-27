#include "VtkRenderWidget.h"

#include "ModelLoader.h"

#include <QDir>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QShowEvent>

#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkCornerAnnotation.h>
#include <vtkTextProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkMapper.h>
#include <vtkProperty.h>
#include <vtkTransform.h>

#include <algorithm>
#include <cmath>

void vtkFpsRenderCallback(vtkObject *, unsigned long eventId, void *clientData, void *)
{
    if (eventId != vtkCommand::EndEvent || !clientData) {
        return;
    }
    static_cast<VtkRenderWidget *>(clientData)->onRenderEnd();
}

namespace {

double lerpValue(double from, double to, double t)
{
    return from + (to - from) * t;
}

double smoothStep(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double randomInRange(double minValue, double maxValue)
{
    return minValue + QRandomGenerator::global()->generateDouble() * (maxValue - minValue);
}

} // namespace

VtkRenderWidget::VtkRenderWidget(QWidget *parent)
    : QWidget(parent)
    , m_roamBoundsValid(false)
    , m_roamProgress(1.0)
    , m_headOpacity(0.55)
    , m_brainOpacity(1.0)
    , m_coilOpacity(1.0)
    , m_simulateIntensity(50)
    , m_simulationEnabled(true)
    , m_roamEnabled(false)
    , m_currentFps(0.0)
    , m_hasFpsSample(false)
    , m_fpsOverlayVisible(true)
    , m_coilModelVisible(false)
    , m_hasInitialCamera(false)
    , m_vtkInitialized(false)
    , m_sceneReady(false)
{
    setMinimumSize(640, 480);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(false);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);

    m_coilTransform = vtkSmartPointer<vtkTransform>::New();
    m_colorMap.setRange(40.0, 150.0);

    m_renderTimer.setSingleShot(true);
    m_renderTimer.setInterval(16);
    connect(&m_renderTimer, &QTimer::timeout, this, &VtkRenderWidget::renderScene);

    m_fieldTimer.setSingleShot(true);
    m_fieldTimer.setInterval(40);
    connect(&m_fieldTimer, &QTimer::timeout, this, &VtkRenderWidget::updateFieldColors);

    m_roamTimer.setInterval(40);
    connect(&m_roamTimer, &QTimer::timeout, this, &VtkRenderWidget::stepRoamAnimation);

    m_fpsElapsedTimer.start();
}

VtkRenderWidget::~VtkRenderWidget()
{
    if (m_renderWindow && m_fpsRenderObserver) {
        m_renderWindow->RemoveObservers(vtkCommand::EndEvent, m_fpsRenderObserver);
    }

    if (m_interactor) {
        m_interactor->TerminateApp();
    }
    if (m_renderWindow) {
        m_renderWindow->Finalize();
    }
}

QPaintEngine *VtkRenderWidget::paintEngine() const
{
    return nullptr;
}

void VtkRenderWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_vtkInitialized) {
        initializeVtk();
    }
    renderScene();
}

void VtkRenderWidget::initializeVtk()
{
    if (m_vtkInitialized) {
        return;
    }

    winId();

    m_renderWindow = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();
    m_renderWindow->SetParentId(reinterpret_cast<void *>(winId()));
    m_renderWindow->SetSize(width() > 0 ? width() : 640, height() > 0 ? height() : 480);
    m_renderWindow->SetWindowName("CTM Viewer");
    m_renderWindow->SetMultiSamples(0);

    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.98, 0.99, 1.00);
    m_renderer->SetBackground2(0.94, 0.96, 0.99);
    m_renderer->GradientBackgroundOn();
    m_renderWindow->AddRenderer(m_renderer);
    setupTransparencyRendering();

    vtkNew<vtkCamera> camera;
    m_renderer->SetActiveCamera(camera.Get());

    m_interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    m_interactor->SetRenderWindow(m_renderWindow);
    m_interactor->SetDesiredUpdateRate(0.0001);
    m_interactor->SetStillUpdateRate(0.0001);
    m_interactor->EnableRenderOn();

    vtkNew<vtkInteractorStyleTrackballCamera> style;
    style->SetAutoAdjustCameraClippingRange(true);
    style->SetMotionFactor(4.0);
    m_interactor->SetInteractorStyle(style.Get());
    m_interactor->Initialize();

    m_renderWindow->Initialize();
    setupFpsObserver();
    setupFpsOverlay();
    m_vtkInitialized = true;
}

void VtkRenderWidget::setupTransparencyRendering()
{
    m_renderWindow->SetAlphaBitPlanes(1);
    m_renderWindow->SetMultiSamples(0);

    m_renderer->SetUseDepthPeeling(1);
    m_renderer->SetMaximumNumberOfPeels(64);
    m_renderer->SetOcclusionRatio(0.05);
}

void VtkRenderWidget::applySurfaceQuality(vtkActor *actor, double opacity, bool offsetCoincident)
{
    if (!actor) {
        return;
    }

    vtkProperty *property = actor->GetProperty();
    property->SetOpacity(opacity);
    property->SetInterpolationToPhong();
    property->SetAmbient(0.22);
    property->SetDiffuse(0.74);
    property->SetSpecular(0.10);
    property->SetSpecularPower(24.0);
    property->BackfaceCullingOff();

    if (opacity < 0.999) {
        property->SetOpacity(opacity);
    } else {
        property->SetOpacity(1.0);
    }

    vtkMapper *mapper = actor->GetMapper();
    if (!mapper) {
        return;
    }

    if (offsetCoincident) {
        mapper->SetResolveCoincidentTopologyToPolygonOffset();
        mapper->SetResolveCoincidentTopologyPolygonOffsetParameters(2.0, 2.0);
    } else {
        mapper->SetResolveCoincidentTopologyToOff();
    }
}

void VtkRenderWidget::setupFpsObserver()
{
    if (m_fpsRenderObserver || !m_renderWindow) {
        return;
    }

    vtkNew<vtkCallbackCommand> callback;
    callback->SetClientData(this);
    callback->SetCallback(vtkFpsRenderCallback);
    m_fpsRenderObserver = callback.Get();
    m_renderWindow->AddObserver(vtkCommand::EndEvent, callback.Get());
}

void VtkRenderWidget::setupFpsOverlay()
{
    if (m_fpsAnnotation) {
        return;
    }

    m_fpsAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
    m_fpsAnnotation->SetLinearFontScaleFactor(2);
    m_fpsAnnotation->SetNonlinearFontScaleFactor(1);
    m_fpsAnnotation->SetMaximumFontSize(14);
    m_fpsAnnotation->GetTextProperty()->SetColor(0.22, 0.28, 0.38);
    m_fpsAnnotation->GetTextProperty()->SetBold(1);
    m_fpsAnnotation->GetTextProperty()->SetShadow(1);
    m_fpsAnnotation->SetText(vtkCornerAnnotation::LowerLeft, "FPS: --");
    m_fpsAnnotation->SetVisibility(m_fpsOverlayVisible ? 1 : 0);
    m_renderer->AddViewProp(m_fpsAnnotation);
}

void VtkRenderWidget::setupCoilAxes()
{
    if (m_coilAxes) {
        return;
    }

    m_coilAxes = vtkSmartPointer<vtkAxesActor>::New();
    m_coilAxes->SetTotalLength(30.0, 30.0, 30.0);
    m_coilAxes->SetShaftTypeToCylinder();
    m_coilAxes->SetCylinderRadius(0.03);
    m_coilAxes->SetConeRadius(0.08);
    m_coilAxes->SetAxisLabels(0);
    m_coilAxes->SetPickable(0);
    m_coilAxes->SetVisibility(m_coilModelVisible ? 0 : 1);
    m_renderer->AddViewProp(m_coilAxes);
}

void VtkRenderWidget::updateCoilVisibility()
{
    if (m_coilActor) {
        m_coilActor->SetVisibility(m_coilModelVisible ? 1 : 0);
    }
    if (m_coilAxes) {
        m_coilAxes->SetVisibility(m_coilModelVisible ? 0 : 1);
    }
    requestRender();
}

void VtkRenderWidget::onRenderEnd()
{
    if (!m_fpsOverlayVisible || !m_fpsAnnotation) {
        return;
    }

    const qint64 deltaMs = m_fpsElapsedTimer.elapsed();

    // Depth peeling triggers multiple EndEvents per visual frame; ignore sub-passes.
    if (deltaMs >= 0 && deltaMs < 12) {
        return;
    }

    if (deltaMs > 500) {
        m_hasFpsSample = false;
        m_currentFps = 0.0;
        m_fpsElapsedTimer.restart();
        m_fpsAnnotation->SetText(vtkCornerAnnotation::LowerLeft, "FPS: --");
        return;
    }

    if (!m_hasFpsSample) {
        m_fpsElapsedTimer.restart();
        m_hasFpsSample = true;
        return;
    }

    if (deltaMs > 0) {
        const double instantFps = std::clamp(1000.0 / static_cast<double>(deltaMs), 1.0, 240.0);
        constexpr double kSmoothing = 0.2;
        m_currentFps = m_currentFps <= 0.0
                           ? instantFps
                           : (m_currentFps * (1.0 - kSmoothing) + instantFps * kSmoothing);
    }

    m_fpsElapsedTimer.restart();

    const QString text = QStringLiteral("FPS: %1").arg(qRound(m_currentFps));
    m_fpsAnnotation->SetText(vtkCornerAnnotation::LowerLeft, text.toUtf8().constData());
}

void VtkRenderWidget::computeRoamBounds(vtkPolyData *headData)
{
    if (!headData) {
        m_roamBoundsValid = false;
        return;
    }

    double bounds[6] = {};
    headData->GetBounds(bounds);

    constexpr double padXY = 45.0;
    constexpr double padZBottom = 15.0;
    constexpr double padZTop = 90.0;

    m_roamBounds[0] = bounds[0] - padXY;
    m_roamBounds[1] = bounds[1] + padXY;
    m_roamBounds[2] = bounds[2] - padXY;
    m_roamBounds[3] = bounds[3] + padXY;
    m_roamBounds[4] = bounds[4] - padZBottom;
    m_roamBounds[5] = bounds[5] + padZTop;
    m_roamBoundsValid = true;
}

void VtkRenderWidget::pickRoamTarget()
{
    if (!m_roamBoundsValid) {
        return;
    }

    m_roamStartPose = m_pose;
    m_roamTargetPose.x = randomInRange(m_roamBounds[0], m_roamBounds[1]);
    m_roamTargetPose.y = randomInRange(m_roamBounds[2], m_roamBounds[3]);
    m_roamTargetPose.z = randomInRange(m_roamBounds[4], m_roamBounds[5]);
    m_roamTargetPose.rx = randomInRange(-55.0, 25.0);
    m_roamTargetPose.ry = randomInRange(-35.0, 35.0);
    m_roamTargetPose.rz = randomInRange(-40.0, 40.0);
    m_roamProgress = 0.0;
}

void VtkRenderWidget::stepRoamAnimation()
{
    if (!m_roamEnabled || !m_sceneReady) {
        return;
    }

    m_roamProgress += 0.04;
    if (m_roamProgress >= 1.0) {
        pickRoamTarget();
    }

    const double t = smoothStep(std::min(m_roamProgress, 1.0));
    CoilPose pose;
    pose.x = lerpValue(m_roamStartPose.x, m_roamTargetPose.x, t);
    pose.y = lerpValue(m_roamStartPose.y, m_roamTargetPose.y, t);
    pose.z = lerpValue(m_roamStartPose.z, m_roamTargetPose.z, t);
    pose.rx = lerpValue(m_roamStartPose.rx, m_roamTargetPose.rx, t);
    pose.ry = lerpValue(m_roamStartPose.ry, m_roamTargetPose.ry, t);
    pose.rz = lerpValue(m_roamStartPose.rz, m_roamTargetPose.rz, t);
    applyCoilPose(pose, true);
}

void VtkRenderWidget::applyCoilPose(const CoilPose &pose, bool emitSignal)
{
    m_pose = pose;
    updateCoilMatrix();
    scheduleFieldUpdate();
    if (emitSignal) {
        emitCoilPose();
    }
}

void VtkRenderWidget::emitCoilPose()
{
    emit coilPoseChanged(m_pose.x, m_pose.y, m_pose.z, m_pose.rx, m_pose.ry, m_pose.rz);
}

bool VtkRenderWidget::loadSimulationScene(const QString &assetRoot, QString *errorMessage)
{
    if (!m_vtkInitialized) {
        initializeVtk();
    }

    const QString fieldPath = QDir(assetRoot).filePath(QStringLiteral("data/b9076_efield.bin"));
    if (!m_fieldSimulator.loadBinary(fieldPath, errorMessage)) {
        return false;
    }

    ModelLoader loader;
    auto loadMesh = [&](const QString &relativePath, vtkSmartPointer<vtkPolyData> &target) -> bool {
        const QString path = QDir(assetRoot).filePath(relativePath);
        QString meshError;
        if (!loader.loadModel(path, &meshError)) {
            if (errorMessage) {
                *errorMessage = meshError;
            }
            return false;
        }
        target = loader.polyData();
        return true;
    };

    vtkSmartPointer<vtkPolyData> headData;
    vtkSmartPointer<vtkPolyData> brainData;
    vtkSmartPointer<vtkPolyData> coilData;
    if (!loadMesh(QStringLiteral("models/head.ctm"), headData)
        || !loadMesh(QStringLiteral("models/brain_mask.ctm"), brainData)
        || !loadMesh(QStringLiteral("models/coil_b090c.ctm"), coilData)) {
        return false;
    }

    if (m_headActor) {
        m_renderer->RemoveActor(m_headActor);
    }
    if (m_brainActor) {
        m_renderer->RemoveActor(m_brainActor);
    }
    if (m_coilActor) {
        m_renderer->RemoveActor(m_coilActor);
    }

    m_brainPolyData = brainData;
    m_fieldSimulator.resetBrainColors(m_brainPolyData);
    computeRoamBounds(headData);

    m_headActor = createActor(headData, 0.42, 0.36, 0.32, m_headOpacity, false);
    applySurfaceQuality(m_headActor, m_headOpacity, true);
    m_brainActor = createActor(m_brainPolyData,
                               FieldColorMap::kBrainGray,
                               FieldColorMap::kBrainGray,
                               FieldColorMap::kBrainGray,
                               m_brainOpacity,
                               true);
    applySurfaceQuality(m_brainActor, m_brainOpacity, false);
    m_coilActor = createActor(coilData, 0.55, 0.78, 0.95, m_coilOpacity, false);
    applySurfaceQuality(m_coilActor, m_coilOpacity, false);

    m_renderer->AddActor(m_headActor);
    m_renderer->AddActor(m_brainActor);
    m_renderer->AddActor(m_coilActor);
    setupCoilAxes();

    m_defaultPose = m_pose;
    updateCoilMatrix();
    updateCoilVisibility();
    updateFieldColors();
    m_renderer->ResetCamera();
    m_renderer->ResetCameraClippingRange();
    captureInitialCamera();
    m_sceneReady = true;
    renderScene();
    return true;
}

vtkSmartPointer<vtkActor> VtkRenderWidget::createActor(vtkPolyData *polyData,
                                                         double r,
                                                         double g,
                                                         double b,
                                                         double opacity,
                                                         bool useVertexColors)
{
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(polyData);
    if (useVertexColors) {
        mapper->SetScalarModeToUsePointData();
        mapper->SetColorModeToDirectScalars();
    } else {
        mapper->ScalarVisibilityOff();
    }
    mapper->StaticOn();

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper.Get());
    actor->GetProperty()->SetColor(r, g, b);
    applySurfaceQuality(actor, opacity, false);
    return actor;
}

void VtkRenderWidget::updateCoilMatrix()
{
    m_coilTransform->Identity();
    m_coilTransform->Translate(m_pose.x, m_pose.y, m_pose.z);
    m_coilTransform->RotateZ(m_pose.rz);
    m_coilTransform->RotateY(m_pose.ry);
    m_coilTransform->RotateX(m_pose.rx);

    if (m_coilActor) {
        m_coilActor->SetUserTransform(m_coilTransform);
    }
    if (m_coilAxes) {
        m_coilAxes->SetUserTransform(m_coilTransform);
    }
}

void VtkRenderWidget::updateFieldColors()
{
    if (!m_sceneReady || !m_brainPolyData) {
        return;
    }

    if (!m_simulationEnabled
        || !FieldSimulator::isCoilWithinRange(m_coilTransform->GetMatrix())) {
        m_fieldSimulator.resetBrainColors(m_brainPolyData);
    } else {
        m_fieldSimulator.applyToBrain(m_brainPolyData,
                                      m_coilTransform->GetMatrix(),
                                      m_simulateIntensity,
                                      m_colorMap);
    }

    if (m_brainActor) {
        m_brainActor->GetMapper()->Modified();
    }
    requestRender();
}

void VtkRenderWidget::scheduleFieldUpdate()
{
    if (!m_fieldTimer.isActive()) {
        m_fieldTimer.start();
    }
}

void VtkRenderWidget::setSimulationEnabled(bool enabled)
{
    m_simulationEnabled = enabled;
    updateFieldColors();
}

void VtkRenderWidget::setSimulateIntensity(int percent)
{
    m_simulateIntensity = qBound(0, percent, 100);
    scheduleFieldUpdate();
}

void VtkRenderWidget::setLutRange(int minValue, int maxValue)
{
    m_colorMap.setRange(minValue, maxValue);
    scheduleFieldUpdate();
}

void VtkRenderWidget::setCoilPose(double x, double y, double z, double rx, double ry, double rz)
{
    if (m_roamEnabled) {
        return;
    }

    CoilPose pose;
    pose.x = x;
    pose.y = y;
    pose.z = z;
    pose.rx = rx;
    pose.ry = ry;
    pose.rz = rz;
    applyCoilPose(pose, false);
}

void VtkRenderWidget::setRoamEnabled(bool enabled)
{
    if (m_roamEnabled == enabled) {
        return;
    }

    m_roamEnabled = enabled;
    if (enabled) {
        pickRoamTarget();
        m_roamTimer.start();
    } else {
        m_roamTimer.stop();
        applyCoilPose(m_defaultPose, true);
    }
}

bool VtkRenderWidget::roamEnabled() const
{
    return m_roamEnabled;
}

VtkRenderWidget::CoilPose VtkRenderWidget::defaultCoilPose() const
{
    return m_defaultPose;
}

VtkRenderWidget::CoilPose VtkRenderWidget::coilPose() const
{
    return m_pose;
}

void VtkRenderWidget::setViewportColors(double bg1[3], double bg2[3])
{
    if (!m_renderer) {
        return;
    }
    m_renderer->SetBackground(bg1[0], bg1[1], bg1[2]);
    m_renderer->SetBackground2(bg2[0], bg2[1], bg2[2]);
    requestRender();
}

void VtkRenderWidget::setFpsOverlayVisible(bool visible)
{
    m_fpsOverlayVisible = visible;
    if (m_fpsAnnotation) {
        m_fpsAnnotation->SetVisibility(visible ? 1 : 0);
    }
    if (visible) {
        m_fpsElapsedTimer.restart();
        m_hasFpsSample = false;
        m_currentFps = 0.0;
        if (m_fpsAnnotation) {
            m_fpsAnnotation->SetText(vtkCornerAnnotation::LowerLeft, "FPS: --");
        }
    }
    requestRender();
}

void VtkRenderWidget::setCoilModelVisible(bool visible)
{
    if (m_coilModelVisible == visible) {
        return;
    }
    m_coilModelVisible = visible;
    updateCoilVisibility();
}

bool VtkRenderWidget::coilModelVisible() const
{
    return m_coilModelVisible;
}

void VtkRenderWidget::setHeadOpacity(double opacity)
{
    m_headOpacity = qBound(0.0, opacity, 1.0);
    if (m_headActor) {
        applySurfaceQuality(m_headActor, m_headOpacity, true);
        requestRender();
    }
}

void VtkRenderWidget::setBrainOpacity(double opacity)
{
    m_brainOpacity = qBound(0.0, opacity, 1.0);
    if (m_brainActor) {
        applySurfaceQuality(m_brainActor, m_brainOpacity, false);
        requestRender();
    }
}

void VtkRenderWidget::setCoilOpacity(double opacity)
{
    m_coilOpacity = qBound(0.0, opacity, 1.0);
    if (m_coilActor) {
        applySurfaceQuality(m_coilActor, m_coilOpacity, false);
        requestRender();
    }
}

void VtkRenderWidget::setModel(vtkSmartPointer<vtkPolyData> polyData)
{
    if (!m_vtkInitialized) {
        initializeVtk();
    }

    if (m_userActor) {
        m_renderer->RemoveActor(m_userActor);
        m_userActor = nullptr;
    }

    m_userPolyData = polyData;
    if (!m_userPolyData || m_userPolyData->GetNumberOfPoints() == 0) {
        requestRender();
        return;
    }

    m_userActor = createActor(m_userPolyData, 0.56, 0.35, 0.21, 1.0, false);
    m_renderer->AddActor(m_userActor);
    m_renderer->ResetCamera();
    m_renderer->ResetCameraClippingRange();
    captureInitialCamera();
    renderScene();
}

void VtkRenderWidget::captureInitialCamera()
{
    if (!m_renderer) {
        return;
    }

    vtkCamera *activeCamera = m_renderer->GetActiveCamera();
    if (!activeCamera) {
        return;
    }

    if (!m_initialCamera) {
        m_initialCamera = vtkSmartPointer<vtkCamera>::New();
    }

    m_initialCamera->DeepCopy(activeCamera);
    m_hasInitialCamera = true;
}

void VtkRenderWidget::setOpacity(double opacity, bool immediate)
{
    setBrainOpacity(opacity);
    Q_UNUSED(immediate)
    emit opacityChanged(opacity);
}

double VtkRenderWidget::opacity() const
{
    return m_brainOpacity;
}

void VtkRenderWidget::resetCamera()
{
    if (!m_renderer) {
        return;
    }

    vtkCamera *activeCamera = m_renderer->GetActiveCamera();
    if (m_hasInitialCamera && m_initialCamera && activeCamera) {
        activeCamera->DeepCopy(m_initialCamera);
        m_renderer->ResetCameraClippingRange();
    } else {
        m_renderer->ResetCamera();
        m_renderer->ResetCameraClippingRange();
    }
    renderScene();
}

void VtkRenderWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_renderWindow && m_vtkInitialized) {
        m_renderWindow->SetSize(event->size().width(), event->size().height());
        requestRender();
    }
}

void VtkRenderWidget::renderScene()
{
    if (m_renderWindow && m_renderer) {
        m_renderWindow->Render();
    }
}

void VtkRenderWidget::requestRender()
{
    if (!m_renderTimer.isActive()) {
        m_renderTimer.start();
    }
}

void VtkRenderWidget::forwardMouseEvent(QMouseEvent *event,
                                        void (vtkRenderWindowInteractor::*handler)())
{
    if (!m_interactor || !handler) {
        return;
    }

    m_interactor->SetEventInformationFlipY(event->x(),
                                           event->y(),
                                           event->modifiers() & Qt::ControlModifier,
                                           event->modifiers() & Qt::ShiftModifier);
    (m_interactor->*handler)();
}

void VtkRenderWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    if (event->button() == Qt::LeftButton) {
        forwardMouseEvent(event, &vtkRenderWindowInteractor::LeftButtonPressEvent);
    } else if (event->button() == Qt::RightButton) {
        forwardMouseEvent(event, &vtkRenderWindowInteractor::RightButtonPressEvent);
    } else if (event->button() == Qt::MiddleButton) {
        forwardMouseEvent(event, &vtkRenderWindowInteractor::MiddleButtonPressEvent);
    }
    event->accept();
}

void VtkRenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() != Qt::NoButton) {
        forwardMouseEvent(event, &vtkRenderWindowInteractor::MouseMoveEvent);
    }
    event->accept();
}

void VtkRenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        forwardMouseEvent(event, &vtkRenderWindowInteractor::LeftButtonReleaseEvent);
    } else if (event->button() == Qt::RightButton) {
        forwardMouseEvent(event, &vtkRenderWindowInteractor::RightButtonReleaseEvent);
    } else if (event->button() == Qt::MiddleButton) {
        forwardMouseEvent(event, &vtkRenderWindowInteractor::MiddleButtonReleaseEvent);
    }
    event->accept();
}

void VtkRenderWidget::wheelEvent(QWheelEvent *event)
{
    if (m_interactor) {
        const QPoint pos = event->pos();
        m_interactor->SetEventInformationFlipY(pos.x(),
                                               pos.y(),
                                               event->modifiers() & Qt::ControlModifier,
                                               event->modifiers() & Qt::ShiftModifier);
        if (event->angleDelta().y() > 0) {
            m_interactor->InvokeEvent(vtkCommand::MouseWheelForwardEvent);
        } else {
            m_interactor->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
        }
    }
    event->accept();
}
