#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QCheckBox;
class QGridLayout;
class QLabel;
class QSlider;
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
    void onResetCamera();
    void onSimulationToggled(bool enabled);
    void onIntensityMoved(int value);
    void onIntensityReleased();
    void onIntensityChanged(int value);
    void onLutMinChanged(int value);
    void onLutMaxChanged(int value);
    void onCoilSliderChanged();
    void onHeadOpacityMoved(int value);
    void onHeadOpacityReleased();
    void onLeftPanelCollapsed(bool collapsed);

private:
    void setupUi();
    QString assetsRoot() const;
    void syncCoilPose();
    void updateCoilLabels();
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
    QCheckBox *m_simulationCheck;
    QSlider *m_intensitySlider;
    QSlider *m_lutMinSlider;
    QSlider *m_lutMaxSlider;
    QSlider *m_headOpacitySlider;
    QSlider *m_coilXSlider;
    QSlider *m_coilYSlider;
    QSlider *m_coilZSlider;
    QSlider *m_coilRxSlider;
    QSlider *m_coilRySlider;
    QSlider *m_coilRzSlider;
    QLabel *m_intensityLabel;
    QLabel *m_lutMinLabel;
    QLabel *m_lutMaxLabel;
    QLabel *m_headOpacityLabel;
    QLabel *m_coilXLabel;
    QLabel *m_coilYLabel;
    QLabel *m_coilZLabel;
    QLabel *m_coilRxLabel;
    QLabel *m_coilRyLabel;
    QLabel *m_coilRzLabel;
    QLabel *m_statusLabel;
    QString m_currentFile;
};

#endif
