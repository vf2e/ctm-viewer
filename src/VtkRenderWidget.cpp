#include "VtkRenderWidget.h"

#include "ModelLoader.h"

#include <QDir>
#include <QResizeEvent>
#include <QShowEvent>

#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkProperty.h>
#include <vtkTransform.h>

VtkRenderWidget::VtkRenderWidget(QWidget *parent)
    : QWidget(parent)
    , m_headOpacity(0.55)
    , m_brainOpacity(0.85)
    , m_coilOpacity(0.88)
    , m_coilX(0.0)
    , m_coilY(0.0)
    , m_coilZ(120.0)
    , m_coilRx(-20.0)
    , m_coilRy(0.0)
    , m_coilRz(0.0)
    , m_simulateIntensity(50)
    , m_simulationEnabled(true)
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
}

VtkRenderWidget::~VtkRenderWidget()
{
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

    vtkNew<vtkCamera> camera;
    m_renderer->SetActiveCamera(camera.Get());

    m_interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    m_interactor->SetRenderWindow(m_renderWindow);
    m_interactor->SetDesiredUpdateRate(60.0);
    m_interactor->SetStillUpdateRate(0.0001);
    m_interactor->EnableRenderOn();

    vtkNew<vtkInteractorStyleTrackballCamera> style;
    style->SetAutoAdjustCameraClippingRange(true);
    m_interactor->SetInteractorStyle(style.Get());
    m_interactor->Initialize();

    m_renderWindow->Initialize();
    m_vtkInitialized = true;
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

    m_headActor = createActor(headData, 0.42, 0.36, 0.32, m_headOpacity, false);
    m_brainActor = createActor(m_brainPolyData, 0.58, 0.58, 0.58, m_brainOpacity, true);
    m_coilActor = createActor(coilData, 0.64, 0.64, 0.64, m_coilOpacity, false);

    m_renderer->AddActor(m_headActor);
    m_renderer->AddActor(m_brainActor);
    m_renderer->AddActor(m_coilActor);

    updateCoilMatrix();
    updateFieldColors();
    m_renderer->ResetCamera();
    m_renderer->ResetCameraClippingRange();
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
    actor->GetProperty()->SetOpacity(opacity);
    actor->GetProperty()->SetInterpolationToPhong();
    actor->GetProperty()->SetAmbient(0.18);
    actor->GetProperty()->SetDiffuse(0.78);
    actor->GetProperty()->SetSpecular(0.12);
    actor->GetProperty()->BackfaceCullingOff();
    return actor;
}

void VtkRenderWidget::updateCoilMatrix()
{
    if (!m_coilActor) {
        return;
    }

    m_coilTransform->Identity();
    m_coilTransform->Translate(m_coilX, m_coilY, m_coilZ);
    m_coilTransform->RotateZ(m_coilRz);
    m_coilTransform->RotateY(m_coilRy);
    m_coilTransform->RotateX(m_coilRx);
    m_coilActor->SetUserTransform(m_coilTransform);
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
    m_coilX = x;
    m_coilY = y;
    m_coilZ = z;
    m_coilRx = rx;
    m_coilRy = ry;
    m_coilRz = rz;
    updateCoilMatrix();
    scheduleFieldUpdate();
}

void VtkRenderWidget::setHeadOpacity(double opacity)
{
    m_headOpacity = qBound(0.0, opacity, 1.0);
    if (m_headActor) {
        m_headActor->GetProperty()->SetOpacity(m_headOpacity);
        requestRender();
    }
}

void VtkRenderWidget::setBrainOpacity(double opacity)
{
    m_brainOpacity = qBound(0.0, opacity, 1.0);
    if (m_brainActor) {
        m_brainActor->GetProperty()->SetOpacity(m_brainOpacity);
        requestRender();
    }
}

void VtkRenderWidget::setCoilOpacity(double opacity)
{
    m_coilOpacity = qBound(0.0, opacity, 1.0);
    if (m_coilActor) {
        m_coilActor->GetProperty()->SetOpacity(m_coilOpacity);
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
    renderScene();
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
    if (m_renderer) {
        m_renderer->ResetCamera();
        m_renderer->ResetCameraClippingRange();
        renderScene();
    }
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
