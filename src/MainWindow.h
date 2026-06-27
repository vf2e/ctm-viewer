#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "AppTheme.h"

#include <QMainWindow>
#include <QString>

class QGridLayout;
class QLabel;
class QPushButton;
class QSlider;
class QSpinBox;
class ColorBarWidget;
class FloatPanel;
class VtkRenderWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void openFile(const QString &filePath);
    bool loadDefaultSimulation(QString *errorMessage = nullptr);
    void setStatusMessage(const QString &message);

private slots:
    void onOpenFile();
    void onOpenSettings();
    void onResetCamera();
    void onSimulationToggled(bool enabled);
    void onFpsOverlayToggled(bool enabled);
    void onCoilModelToggled(bool enabled);
    void onRoamToggled(bool enabled);
    void onIntensityMoved(int value);
    void onIntensityReleased();
    void onIntensityChanged(int value);
    void onLutMinChanged(int value);
    void onLutMaxChanged(int value);
    void onCoilSliderChanged();
    void onCoilPoseFromViewer(double x, double y, double z, double rx, double ry, double rz);
    void onHeadOpacityMoved(int value);
    void onHeadOpacityReleased();
    void onLeftPanelCollapsed(bool collapsed);
    void onRightPanelCollapsed(bool collapsed);

private:
    void setupUi();
    void setupShortcuts();
    void applyTheme(AppTheme::ThemeId theme);
    void applyLutRange(int minValue, int maxValue);
    void setCoilControlsEnabled(bool enabled);
    QString assetsRoot() const;
    void syncCoilPose();
    void updateCoilLabels();
    void setCoilSlidersFromPose(double x, double y, double z, double rx, double ry, double rz);
    QSlider *addSliderRow(QGridLayout *grid,
                          int row,
                          const QString &label,
                          int minValue,
                          int maxValue,
                          int value,
                          QLabel **valueLabelOut);
    void updateStatus(const QString &filePath, int vertices, int triangles);
    void showError(const QString &message);

    VtkRenderWidget *m_viewer;
    FloatPanel *m_leftPanel;
    FloatPanel *m_colorPanel;
    ColorBarWidget *m_colorBar;
    QPushButton *m_simulationToggle;
    QPushButton *m_fpsToggle;
    QPushButton *m_coilModelToggle;
    QPushButton *m_roamButton;
    QSlider *m_intensitySlider;
    QSpinBox *m_lutMinSpin;
    QSpinBox *m_lutMaxSpin;
    QSlider *m_headOpacitySlider;
    QSlider *m_coilXSlider;
    QSlider *m_coilYSlider;
    QSlider *m_coilZSlider;
    QSlider *m_coilRxSlider;
    QSlider *m_coilRySlider;
    QSlider *m_coilRzSlider;
    QLabel *m_intensityLabel;
    QLabel *m_headOpacityLabel;
    QLabel *m_coilXLabel;
    QLabel *m_coilYLabel;
    QLabel *m_coilZLabel;
    QLabel *m_coilRxLabel;
    QLabel *m_coilRyLabel;
    QLabel *m_coilRzLabel;
    QLabel *m_statusLabel;
    AppTheme::ThemeId m_currentTheme;
    QString m_currentFile;
};

#endif
