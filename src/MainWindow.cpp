#include "MainWindow.h"

#include "AppTheme.h"
#include "ColorBarWidget.h"
#include "FloatPanel.h"
#include "ModelLoader.h"
#include "SettingsDialog.h"
#include "VtkRenderWidget.h"

#include <QAbstractSpinBox>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QVBoxLayout>

#include <cmath>

namespace {

QIcon applicationIcon()
{
    QIcon icon(QStringLiteral(":/icon.ico"));
    if (!icon.isNull()) {
        return icon;
    }

    const QString fileIcon = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("icon.ico"));
    if (QFileInfo::exists(fileIcon)) {
        return QIcon(fileIcon);
    }

    return QIcon();
}

QSpinBox *createLutSpinBox(int value, QWidget *parent)
{
    auto *spin = new QSpinBox(parent);
    spin->setObjectName(QStringLiteral("flatField"));
    spin->setRange(1, 999);
    spin->setValue(value);
    spin->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spin->setFixedHeight(38);
    return spin;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_currentTheme(AppTheme::ThemeId::Light)
{
    setWindowTitle(QStringLiteral("CTM Viewer · 八字拍电场仿真"));
    setWindowIcon(applicationIcon());
    resize(1440, 900);
    setupUi();
    setupShortcuts();
    applyTheme(AppTheme::ThemeId::Light);
}

QString MainWindow::assetsRoot() const
{
    const QString bundled = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("assets"));
    if (QDir(bundled).exists()) {
        return bundled;
    }
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../assets"));
}

bool MainWindow::loadDefaultSimulation(QString *errorMessage)
{
    if (m_viewer->loadSimulationScene(assetsRoot(), errorMessage)) {
        m_colorBar->setRange(m_lutMinSpin->value(), m_lutMaxSpin->value());
        const auto pose = m_viewer->defaultCoilPose();
        setCoilSlidersFromPose(pose.x, pose.y, pose.z, pose.rx, pose.ry, pose.rz);
        m_viewer->setCoilModelVisible(false);
        m_viewer->setFpsOverlayVisible(m_fpsToggle->isChecked());
        m_statusLabel->setText(QStringLiteral("电场仿真就绪 · 蓝红连续热力图"));
        return true;
    }
    return false;
}

void MainWindow::setStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
}

void MainWindow::applyTheme(AppTheme::ThemeId theme)
{
    m_currentTheme = theme;
    setStyleSheet(AppTheme::stylesheet(theme));

    const AppTheme::ViewportColors colors = AppTheme::viewportColors(theme);
    m_viewer->setViewportColors(const_cast<double *>(colors.bg1), const_cast<double *>(colors.bg2));
}

QSlider *MainWindow::addSliderRow(QGridLayout *grid,
                                  int row,
                                  const QString &label,
                                  int minValue,
                                  int maxValue,
                                  int value,
                                  QLabel **valueLabelOut)
{
    auto *name = new QLabel(label);
    name->setObjectName(QStringLiteral("fieldLabel"));

    auto *slider = new QSlider(Qt::Horizontal);
    slider->setObjectName(QStringLiteral("accentSlider"));
    slider->setRange(minValue, maxValue);
    slider->setValue(value);

    auto *valueLabel = new QLabel(QString::number(value));
    valueLabel->setObjectName(QStringLiteral("valueLabel"));
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    grid->addWidget(name, row, 0);
    grid->addWidget(slider, row, 1);
    grid->addWidget(valueLabel, row, 2);

    if (valueLabelOut) {
        *valueLabelOut = valueLabel;
    }
    return slider;
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(12);

    m_leftPanel = new FloatPanel(QStringLiteral("调试面板"),
                                 QStringLiteral("调试"),
                                 FloatPanel::Side::Left,
                                 central);
    m_leftPanel->setFixedWidth(300);
    connect(m_leftPanel, &FloatPanel::collapsedChanged, this, &MainWindow::onLeftPanelCollapsed);

    auto *leftContent = m_leftPanel->contentLayout();

    auto *actions = new QHBoxLayout();
    actions->setSpacing(8);

    auto *openButton = new QPushButton(QStringLiteral("打开"));
    openButton->setObjectName(QStringLiteral("primaryBtn"));
    connect(openButton, &QPushButton::clicked, this, &MainWindow::onOpenFile);

    auto *settingsButton = new QPushButton(QStringLiteral("设置"));
    settingsButton->setObjectName(QStringLiteral("secondaryBtn"));
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::onOpenSettings);

    auto *resetButton = new QPushButton(QStringLiteral("重置视角"));
    resetButton->setObjectName(QStringLiteral("secondaryBtn"));
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onResetCamera);

    actions->addWidget(openButton);
    actions->addWidget(settingsButton);
    actions->addWidget(resetButton);
    leftContent->addLayout(actions);

    m_roamButton = new QPushButton(QStringLiteral("漫游模式"));
    m_roamButton->setObjectName(QStringLiteral("roamBtn"));
    m_roamButton->setCheckable(true);
    m_roamButton->setCursor(Qt::PointingHandCursor);
    connect(m_roamButton, &QPushButton::toggled, this, &MainWindow::onRoamToggled);
    leftContent->addWidget(m_roamButton);

    auto *divider = new QFrame();
    divider->setObjectName(QStringLiteral("sectionDivider"));
    divider->setFrameShape(QFrame::HLine);
    leftContent->addWidget(divider);

    auto *displayTitle = new QLabel(QStringLiteral("显示选项"));
    displayTitle->setObjectName(QStringLiteral("groupTitle"));
    leftContent->addWidget(displayTitle);

    m_fpsToggle = new QPushButton(QStringLiteral("显示帧率"));
    m_fpsToggle->setObjectName(QStringLiteral("toggleBtn"));
    m_fpsToggle->setCheckable(true);
    m_fpsToggle->setChecked(true);
    m_fpsToggle->setCursor(Qt::PointingHandCursor);
    connect(m_fpsToggle, &QPushButton::toggled, this, &MainWindow::onFpsOverlayToggled);
    leftContent->addWidget(m_fpsToggle);

    m_coilModelToggle = new QPushButton(QStringLiteral("显示线圈拍模型"));
    m_coilModelToggle->setObjectName(QStringLiteral("toggleBtn"));
    m_coilModelToggle->setCheckable(true);
    m_coilModelToggle->setChecked(false);
    m_coilModelToggle->setCursor(Qt::PointingHandCursor);
    connect(m_coilModelToggle, &QPushButton::toggled, this, &MainWindow::onCoilModelToggled);
    leftContent->addWidget(m_coilModelToggle);

    m_simulationToggle = new QPushButton(QStringLiteral("启用电场仿真"));
    m_simulationToggle->setObjectName(QStringLiteral("toggleBtn"));
    m_simulationToggle->setCheckable(true);
    m_simulationToggle->setChecked(true);
    m_simulationToggle->setCursor(Qt::PointingHandCursor);
    connect(m_simulationToggle, &QPushButton::toggled, this, &MainWindow::onSimulationToggled);
    leftContent->addWidget(m_simulationToggle);

    auto *simDivider = new QFrame();
    simDivider->setObjectName(QStringLiteral("sectionDivider"));
    simDivider->setFrameShape(QFrame::HLine);
    leftContent->addWidget(simDivider);

    auto *simTitle = new QLabel(QStringLiteral("仿真参数"));
    simTitle->setObjectName(QStringLiteral("groupTitle"));
    leftContent->addWidget(simTitle);

    auto *simGrid = new QGridLayout();
    simGrid->setHorizontalSpacing(8);
    simGrid->setVerticalSpacing(6);
    simGrid->setColumnStretch(1, 1);

    m_intensitySlider = new QSlider(Qt::Horizontal);
    m_intensitySlider->setObjectName(QStringLiteral("primarySlider"));
    m_intensitySlider->setRange(0, 100);
    m_intensitySlider->setValue(50);
    m_intensityLabel = new QLabel(QStringLiteral("50%"));
    m_intensityLabel->setObjectName(QStringLiteral("valueLabel"));
    connect(m_intensitySlider, &QSlider::sliderMoved, this, &MainWindow::onIntensityMoved);
    connect(m_intensitySlider, &QSlider::sliderReleased, this, &MainWindow::onIntensityReleased);
    connect(m_intensitySlider, &QSlider::valueChanged, this, &MainWindow::onIntensityChanged);

    auto *intensityName = new QLabel(QStringLiteral("强度"));
    intensityName->setObjectName(QStringLiteral("fieldLabel"));
    simGrid->addWidget(intensityName, 0, 0);
    simGrid->addWidget(m_intensitySlider, 0, 1);
    simGrid->addWidget(m_intensityLabel, 0, 2);

    m_headOpacitySlider = new QSlider(Qt::Horizontal);
    m_headOpacitySlider->setObjectName(QStringLiteral("accentSlider"));
    m_headOpacitySlider->setRange(0, 100);
    m_headOpacitySlider->setValue(50);
    m_headOpacityLabel = new QLabel(QStringLiteral("50%"));
    m_headOpacityLabel->setObjectName(QStringLiteral("valueLabel"));
    connect(m_headOpacitySlider, &QSlider::sliderMoved, this, &MainWindow::onHeadOpacityMoved);
    connect(m_headOpacitySlider, &QSlider::sliderReleased, this, &MainWindow::onHeadOpacityReleased);

    auto *headName = new QLabel(QStringLiteral("头皮"));
    headName->setObjectName(QStringLiteral("fieldLabel"));
    simGrid->addWidget(headName, 1, 0);
    simGrid->addWidget(m_headOpacitySlider, 1, 1);
    simGrid->addWidget(m_headOpacityLabel, 1, 2);

    leftContent->addLayout(simGrid);

    auto *coilTitle = new QLabel(QStringLiteral("线圈拍位置 / 角度"));
    coilTitle->setObjectName(QStringLiteral("groupTitle"));
    leftContent->addWidget(coilTitle);

    auto *coilGrid = new QGridLayout();
    coilGrid->setHorizontalSpacing(8);
    coilGrid->setVerticalSpacing(6);
    coilGrid->setColumnStretch(1, 1);

    m_coilXSlider = addSliderRow(coilGrid, 0, QStringLiteral("X"), -120, 120, 0, &m_coilXLabel);
    m_coilYSlider = addSliderRow(coilGrid, 1, QStringLiteral("Y"), -120, 120, 0, &m_coilYLabel);
    m_coilZSlider = addSliderRow(coilGrid, 2, QStringLiteral("Z"), 20, 260, 120, &m_coilZLabel);
    m_coilRxSlider = addSliderRow(coilGrid, 3, QStringLiteral("Rx"), -90, 90, -20, &m_coilRxLabel);
    m_coilRySlider = addSliderRow(coilGrid, 4, QStringLiteral("Ry"), -90, 90, 0, &m_coilRyLabel);
    m_coilRzSlider = addSliderRow(coilGrid, 5, QStringLiteral("Rz"), -90, 90, 0, &m_coilRzLabel);

    const QList<QSlider *> coilSliders = {
        m_coilXSlider, m_coilYSlider, m_coilZSlider,
        m_coilRxSlider, m_coilRySlider, m_coilRzSlider
    };
    for (QSlider *slider : coilSliders) {
        connect(slider, &QSlider::sliderMoved, this, &MainWindow::onCoilSliderChanged);
        connect(slider, &QSlider::sliderReleased, this, &MainWindow::onCoilSliderChanged);
        connect(slider, &QSlider::valueChanged, this, [this, slider]() {
            if (!slider->isSliderDown()) {
                onCoilSliderChanged();
            }
        });
    }

    leftContent->addLayout(coilGrid);
    leftContent->addStretch(1);

    auto *center = new QWidget(central);
    auto *centerLayout = new QVBoxLayout(center);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    m_viewer = new VtkRenderWidget(center);
    centerLayout->addWidget(m_viewer, 1);
    connect(m_viewer, &VtkRenderWidget::coilPoseChanged, this, &MainWindow::onCoilPoseFromViewer);

    m_colorPanel = new FloatPanel(QStringLiteral("Colorbar"),
                                  QStringLiteral("色标"),
                                  FloatPanel::Side::Right,
                                  central);
    m_colorPanel->setFixedWidth(168);
    connect(m_colorPanel, &FloatPanel::collapsedChanged, this, &MainWindow::onRightPanelCollapsed);

    auto *colorContent = m_colorPanel->contentLayout();
    auto *unit = new QLabel(QStringLiteral("V/m"));
    unit->setObjectName(QStringLiteral("unitLabel"));
    unit->setAlignment(Qt::AlignCenter);
    colorContent->addWidget(unit);

    m_colorBar = new ColorBarWidget(m_colorPanel);
    colorContent->addWidget(m_colorBar, 1);

    auto *lutTitle = new QLabel(QStringLiteral("色标范围"));
    lutTitle->setObjectName(QStringLiteral("groupTitle"));
    colorContent->addWidget(lutTitle);

    m_lutMaxSpin = createLutSpinBox(150, m_colorPanel);
    connect(m_lutMaxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onLutMaxChanged);

    m_lutMinSpin = createLutSpinBox(40, m_colorPanel);
    connect(m_lutMinSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onLutMinChanged);

    auto *maxName = new QLabel(QStringLiteral("上限"));
    maxName->setObjectName(QStringLiteral("fieldLabel"));
    auto *minName = new QLabel(QStringLiteral("下限"));
    minName->setObjectName(QStringLiteral("fieldLabel"));
    auto *unitMax = new QLabel(QStringLiteral("V/m"));
    unitMax->setObjectName(QStringLiteral("unitLabel"));
    auto *unitMin = new QLabel(QStringLiteral("V/m"));
    unitMin->setObjectName(QStringLiteral("unitLabel"));

    auto *lutGrid = new QGridLayout();
    lutGrid->setHorizontalSpacing(8);
    lutGrid->setVerticalSpacing(10);
    lutGrid->setColumnStretch(1, 1);
    lutGrid->addWidget(maxName, 0, 0);
    lutGrid->addWidget(m_lutMaxSpin, 0, 1);
    lutGrid->addWidget(unitMax, 0, 2);
    lutGrid->addWidget(minName, 1, 0);
    lutGrid->addWidget(m_lutMinSpin, 1, 1);
    lutGrid->addWidget(unitMin, 1, 2);
    colorContent->addLayout(lutGrid);

    applyLutRange(m_lutMinSpin->value(), m_lutMaxSpin->value());

    root->addWidget(m_leftPanel);
    root->addWidget(center, 1);
    root->addWidget(m_colorPanel);
    setCentralWidget(central);

    m_statusLabel = new QLabel(QStringLiteral("正在加载仿真场景..."));
    statusBar()->addPermanentWidget(m_statusLabel, 1);
}

void MainWindow::setupShortcuts()
{
    auto *openShortcut = new QShortcut(QKeySequence::Open, this);
    connect(openShortcut, &QShortcut::activated, this, &MainWindow::onOpenFile);

    auto *resetShortcut = new QShortcut(QKeySequence(Qt::Key_Home), this);
    connect(resetShortcut, &QShortcut::activated, this, &MainWindow::onResetCamera);
}

void MainWindow::onOpenSettings()
{
    QWidget *central = centralWidget();
    if (!central) {
        return;
    }

    auto *overlay = new QWidget(central);
    overlay->setObjectName(QStringLiteral("modalOverlay"));
    overlay->setGeometry(central->rect());
    overlay->show();
    overlay->raise();

    SettingsDialog dialog(m_currentTheme, this);
    const QRect host = frameGeometry();
    dialog.move(host.center() - QPoint(dialog.width() / 2, dialog.height() / 2));

    const int result = dialog.exec();
    overlay->deleteLater();

    if (result != QDialog::Accepted) {
        return;
    }

    applyTheme(dialog.selectedTheme());
}

void MainWindow::applyLutRange(int minValue, int maxValue)
{
    m_colorBar->setRange(minValue, maxValue);
    m_viewer->setLutRange(minValue, maxValue);
}

void MainWindow::onLeftPanelCollapsed(bool collapsed)
{
    m_leftPanel->setFixedWidth(collapsed ? 44 : 300);
}

void MainWindow::onRightPanelCollapsed(bool collapsed)
{
    m_colorPanel->setFixedWidth(collapsed ? 44 : 168);
}

void MainWindow::setCoilControlsEnabled(bool enabled)
{
    const QList<QSlider *> coilSliders = {
        m_coilXSlider, m_coilYSlider, m_coilZSlider,
        m_coilRxSlider, m_coilRySlider, m_coilRzSlider
    };
    for (QSlider *slider : coilSliders) {
        slider->setEnabled(enabled);
    }
}

void MainWindow::setCoilSlidersFromPose(double x, double y, double z, double rx, double ry, double rz)
{
    const QList<QSlider *> sliders = {
        m_coilXSlider, m_coilYSlider, m_coilZSlider,
        m_coilRxSlider, m_coilRySlider, m_coilRzSlider
    };
    const QList<int> values = {
        static_cast<int>(std::lround(x)),
        static_cast<int>(std::lround(y)),
        static_cast<int>(std::lround(z)),
        static_cast<int>(std::lround(rx)),
        static_cast<int>(std::lround(ry)),
        static_cast<int>(std::lround(rz)),
    };

    for (int i = 0; i < sliders.size(); ++i) {
        sliders[i]->blockSignals(true);
        sliders[i]->setValue(values[i]);
        sliders[i]->blockSignals(false);
    }
    updateCoilLabels();
}

void MainWindow::syncCoilPose()
{
    m_viewer->setCoilPose(m_coilXSlider->value(),
                          m_coilYSlider->value(),
                          m_coilZSlider->value(),
                          m_coilRxSlider->value(),
                          m_coilRySlider->value(),
                          m_coilRzSlider->value());
}

void MainWindow::updateCoilLabels()
{
    m_coilXLabel->setText(QString::number(m_coilXSlider->value()));
    m_coilYLabel->setText(QString::number(m_coilYSlider->value()));
    m_coilZLabel->setText(QString::number(m_coilZSlider->value()));
    m_coilRxLabel->setText(QString::number(m_coilRxSlider->value()));
    m_coilRyLabel->setText(QString::number(m_coilRySlider->value()));
    m_coilRzLabel->setText(QString::number(m_coilRzSlider->value()));
}

void MainWindow::onCoilSliderChanged()
{
    updateCoilLabels();
    syncCoilPose();
}

void MainWindow::onCoilPoseFromViewer(double x, double y, double z, double rx, double ry, double rz)
{
    setCoilSlidersFromPose(x, y, z, rx, ry, rz);
}

void MainWindow::onRoamToggled(bool enabled)
{
    m_viewer->setRoamEnabled(enabled);
    setCoilControlsEnabled(!enabled);
    if (enabled) {
        m_statusLabel->setText(QStringLiteral("漫游模式已开启 · 线圈拍在头模附近随机运动"));
    } else {
        m_statusLabel->setText(QStringLiteral("漫游模式已关闭 · 线圈拍已复位"));
    }
}

void MainWindow::onOpenFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("打开模型"),
        m_currentFile.isEmpty() ? QString() : QFileInfo(m_currentFile).absolutePath(),
        QStringLiteral("3D 模型 (*.ctm *.obj);;所有文件 (*.*)"));
    if (!filePath.isEmpty()) {
        openFile(filePath);
    }
}

void MainWindow::openFile(const QString &filePath)
{
    ModelLoader loader;
    QString error;
    if (!loader.loadModel(filePath, &error)) {
        showError(error.isEmpty() ? QStringLiteral("模型加载失败") : error);
        return;
    }

    m_currentFile = filePath;
    m_viewer->setModel(loader.polyData());
    updateStatus(filePath, loader.vertexCount(), loader.triangleCount());
}

void MainWindow::onResetCamera()
{
    m_viewer->resetCamera();
}

void MainWindow::onSimulationToggled(bool enabled)
{
    m_viewer->setSimulationEnabled(enabled);
}

void MainWindow::onFpsOverlayToggled(bool enabled)
{
    m_viewer->setFpsOverlayVisible(enabled);
}

void MainWindow::onCoilModelToggled(bool enabled)
{
    m_viewer->setCoilModelVisible(enabled);
}

void MainWindow::onIntensityMoved(int value)
{
    m_intensityLabel->setText(QStringLiteral("%1%").arg(value));
    m_viewer->setSimulateIntensity(value);
}

void MainWindow::onIntensityReleased()
{
    m_viewer->setSimulateIntensity(m_intensitySlider->value());
}

void MainWindow::onIntensityChanged(int value)
{
    m_intensityLabel->setText(QStringLiteral("%1%").arg(value));
    if (!m_intensitySlider->isSliderDown()) {
        m_viewer->setSimulateIntensity(value);
    }
}

void MainWindow::onLutMinChanged(int value)
{
    if (value >= m_lutMaxSpin->value()) {
        m_lutMinSpin->blockSignals(true);
        m_lutMinSpin->setValue(m_lutMaxSpin->value() - 1);
        m_lutMinSpin->blockSignals(false);
        value = m_lutMinSpin->value();
    }
    applyLutRange(value, m_lutMaxSpin->value());
}

void MainWindow::onLutMaxChanged(int value)
{
    if (value <= m_lutMinSpin->value()) {
        m_lutMaxSpin->blockSignals(true);
        m_lutMaxSpin->setValue(m_lutMinSpin->value() + 1);
        m_lutMaxSpin->blockSignals(false);
        value = m_lutMaxSpin->value();
    }
    applyLutRange(m_lutMinSpin->value(), value);
}

void MainWindow::onHeadOpacityMoved(int value)
{
    m_headOpacityLabel->setText(QStringLiteral("%1%").arg(value));
    m_viewer->setHeadOpacity(value / 100.0);
}

void MainWindow::onHeadOpacityReleased()
{
    m_viewer->setHeadOpacity(m_headOpacitySlider->value() / 100.0);
}

void MainWindow::updateStatus(const QString &filePath, int vertices, int triangles)
{
    const QString name = QFileInfo(filePath).fileName();
    m_statusLabel->setText(
        QStringLiteral("附加模型 %1  ·  顶点 %2  ·  三角面 %3")
            .arg(name)
            .arg(vertices)
            .arg(triangles));
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::warning(this, QStringLiteral("错误"), message);
}
