#ifndef VTKRENDERWIDGET_H
#define VTKRENDERWIDGET_H

#include "FieldColorMap.h"
#include "FieldSimulator.h"

#include <QMouseEvent>
#include <QShowEvent>
#include <QTimer>
#include <QWidget>
#include <QWheelEvent>

#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkWin32OpenGLRenderWindow.h>

class VtkRenderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VtkRenderWidget(QWidget *parent = nullptr);
    ~VtkRenderWidget() override;

    bool loadSimulationScene(const QString &assetRoot, QString *errorMessage = nullptr);
    void setModel(vtkSmartPointer<vtkPolyData> polyData);
    void setSimulationEnabled(bool enabled);
    void setSimulateIntensity(int percent);
    void setLutRange(int minValue, int maxValue);
    void setCoilPose(double x, double y, double z, double rx, double ry, double rz);
    void setHeadOpacity(double opacity);
    void setBrainOpacity(double opacity);
    void setCoilOpacity(double opacity);
    void setOpacity(double opacity, bool immediate = true);
    double opacity() const;
    void resetCamera();

signals:
    void opacityChanged(double opacity);

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

    vtkSmartPointer<vtkWin32OpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactor;
    vtkSmartPointer<vtkActor> m_headActor;
    vtkSmartPointer<vtkActor> m_brainActor;
    vtkSmartPointer<vtkActor> m_coilActor;
    vtkSmartPointer<vtkActor> m_userActor;
    vtkSmartPointer<vtkPolyData> m_brainPolyData;
    vtkSmartPointer<vtkPolyData> m_userPolyData;
    vtkSmartPointer<vtkTransform> m_coilTransform;
    FieldSimulator m_fieldSimulator;
    FieldColorMap m_colorMap;
    QTimer m_renderTimer;
    QTimer m_fieldTimer;
    double m_headOpacity;
    double m_brainOpacity;
    double m_coilOpacity;
    double m_coilX;
    double m_coilY;
    double m_coilZ;
    double m_coilRx;
    double m_coilRy;
    double m_coilRz;
    int m_simulateIntensity;
    bool m_simulationEnabled;
    bool m_vtkInitialized;
    bool m_sceneReady;
};

#endif
