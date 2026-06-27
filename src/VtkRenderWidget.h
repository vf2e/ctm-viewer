#ifndef VTKRENDERWIDGET_H
#define VTKRENDERWIDGET_H

#include "FieldColorMap.h"
#include "FieldSimulator.h"

#include <QMouseEvent>
#include <QShowEvent>
#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
#include <QWheelEvent>

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkCornerAnnotation.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkWin32OpenGLRenderWindow.h>

void vtkFpsRenderCallback(vtkObject *, unsigned long eventId, void *clientData, void *);

class VtkRenderWidget : public QWidget
{
    Q_OBJECT

    friend void vtkFpsRenderCallback(vtkObject *, unsigned long, void *, void *);

public:
    struct CoilPose {
        double x = 0.0;
        double y = 0.0;
        double z = 120.0;
        double rx = -20.0;
        double ry = 0.0;
        double rz = 0.0;
    };

    explicit VtkRenderWidget(QWidget *parent = nullptr);
    ~VtkRenderWidget() override;

    bool loadSimulationScene(const QString &assetRoot, QString *errorMessage = nullptr);
    void setModel(vtkSmartPointer<vtkPolyData> polyData);
    void setSimulationEnabled(bool enabled);
    void setSimulateIntensity(int percent);
    void setLutRange(int minValue, int maxValue);
    void setCoilPose(double x, double y, double z, double rx, double ry, double rz);
    void setRoamEnabled(bool enabled);
    bool roamEnabled() const;
    CoilPose defaultCoilPose() const;
    CoilPose coilPose() const;
    void setHeadOpacity(double opacity);
    void setBrainOpacity(double opacity);
    void setCoilOpacity(double opacity);
    void setOpacity(double opacity, bool immediate = true);
    void setViewportColors(double bg1[3], double bg2[3]);
    void setFpsOverlayVisible(bool visible);
    void setCoilModelVisible(bool visible);
    bool coilModelVisible() const;
    double opacity() const;
    void resetCamera();

signals:
    void opacityChanged(double opacity);
    void coilPoseChanged(double x, double y, double z, double rx, double ry, double rz);

protected:
    QPaintEngine *paintEngine() const override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void initializeVtk();
    void renderScene();
    void requestRender();
    void forwardMouseEvent(QMouseEvent *event, void (vtkRenderWindowInteractor::*handler)());
    vtkSmartPointer<vtkActor> createActor(vtkPolyData *polyData,
                                          double r,
                                          double g,
                                          double b,
                                          double opacity,
                                          bool useVertexColors = false);
    void updateFieldColors();
    void scheduleFieldUpdate();
    void updateCoilMatrix();
    void emitCoilPose();
    void computeRoamBounds(vtkPolyData *headData);
    void pickRoamTarget();
    void stepRoamAnimation();
    void applyCoilPose(const CoilPose &pose, bool emitSignal);

    void captureInitialCamera();
    void setupCoilAxes();
    void setupFpsOverlay();
    void setupFpsObserver();
    void setupTransparencyRendering();
    void applySurfaceQuality(vtkActor *actor, double opacity, bool offsetCoincident);
    void updateCoilVisibility();
    void onRenderEnd();

    vtkSmartPointer<vtkWin32OpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkCamera> m_initialCamera;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactor;
    vtkSmartPointer<vtkActor> m_headActor;
    vtkSmartPointer<vtkActor> m_brainActor;
    vtkSmartPointer<vtkActor> m_coilActor;
    vtkSmartPointer<vtkAxesActor> m_coilAxes;
    vtkSmartPointer<vtkCornerAnnotation> m_fpsAnnotation;
    vtkSmartPointer<vtkCommand> m_fpsRenderObserver;
    vtkSmartPointer<vtkActor> m_userActor;
    vtkSmartPointer<vtkPolyData> m_brainPolyData;
    vtkSmartPointer<vtkPolyData> m_userPolyData;
    vtkSmartPointer<vtkTransform> m_coilTransform;
    FieldSimulator m_fieldSimulator;
    FieldColorMap m_colorMap;
    QTimer m_renderTimer;
    QTimer m_fieldTimer;
    QTimer m_roamTimer;
    QElapsedTimer m_fpsElapsedTimer;
    CoilPose m_pose;
    CoilPose m_defaultPose;
    CoilPose m_roamStartPose;
    CoilPose m_roamTargetPose;
    double m_roamBounds[6];
    bool m_roamBoundsValid;
    double m_roamProgress;
    double m_currentFps;
    double m_headOpacity;
    double m_brainOpacity;
    double m_coilOpacity;
    int m_simulateIntensity;
    bool m_hasFpsSample;
    bool m_simulationEnabled;
    bool m_roamEnabled;
    bool m_fpsOverlayVisible;
    bool m_coilModelVisible;
    bool m_hasInitialCamera;
    bool m_vtkInitialized;
    bool m_sceneReady;
};

#endif
