#include "MainWindow.h"

#include "AppStyle.h"
#include "ColorBarWidget.h"
#include "FloatPanel.h"
#include "ModelLoader.h"
#include "VtkRenderWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("CTM Viewer · 八字拍电场仿真"));
    resize(1440, 900);
    setStyleSheet(AppStyle::stylesheet());
    setupUi();
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
        m_statusLabel->setText(QStringLiteral("电场仿真就绪 · 脑模默认亮灰"));
        return true;
    }
    return false;
}

void MainWindow::setStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
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
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(10);

    m_leftPanel = new FloatPanel(QStringLiteral("调试面板"), central);
    m_leftPanel->setFixedWidth(292);
    connect(m_leftPanel, &FloatPanel::collapsedChanged, this, &MainWindow::onLeftPanelCollapsed);

    auto *leftContent = m_leftPanel->contentLayout();

    auto *actions = new QHBoxLayout();
    auto *openButton = new QPushButton(QStringLiteral("打开"));
    openButton->setObjectName(QStringLiteral("primaryBtn"));
    connect(openButton, &QPushButton::clicked, this, &MainWindow::onOpenFile);

    auto *resetButton = new QPushButton(QStringLiteral("重置视角"));
    resetButton->setObjectName(QStringLiteral("secondaryBtn"));
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onResetCamera);

    actions->addWidget(openButton);
    actions->addWidget(resetButton);
    leftContent->addLayout(actions);

    m_simulationCheck = new QCheckBox(QStringLiteral("启用电场仿真"));
    m_simulationCheck->setChecked(true);
    connect(m_simulationCheck, &QCheckBox::toggled, this, &MainWindow::onSimulationToggled);
    leftContent->addWidget(m_simulationCheck);

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

    m_colorPanel = new FloatPanel(QStringLiteral("Colorbar"), central);
    m_colorPanel->setFixedWidth(118);

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

    m_lutMaxSlider = new QSlider(Qt::Horizontal);
    m_lutMaxSlider->setObjectName(QStringLiteral("accentSlider"));
    m_lutMaxSlider->setRange(60, 200);
    m_lutMaxSlider->setValue(150);
    m_lutMaxLabel = new QLabel(QStringLiteral("150"));
    m_lutMaxLabel->setObjectName(QStringLiteral("valueLabel"));
    connect(m_lutMaxSlider, &QSlider::valueChanged, this, &MainWindow::onLutMaxChanged);

    m_lutMinSlider = new QSlider(Qt::Horizontal);
    m_lutMinSlider->setObjectName(QStringLiteral("accentSlider"));
    m_lutMinSlider->setRange(20, 140);
    m_lutMinSlider->setValue(40);
    m_lutMinLabel = new QLabel(QStringLiteral("40"));
    m_lutMinLabel->setObjectName(QStringLiteral("valueLabel"));
    connect(m_lutMinSlider, &QSlider::valueChanged, this, &MainWindow::onLutMinChanged);

    auto *maxName = new QLabel(QStringLiteral("上限"));
    maxName->setObjectName(QStringLiteral("fieldLabel"));
    auto *minName = new QLabel(QStringLiteral("下限"));
    minName->setObjectName(QStringLiteral("fieldLabel"));

    auto *lutGrid = new QGridLayout();
    lutGrid->setHorizontalSpacing(4);
    lutGrid->setVerticalSpacing(6);
    lutGrid->addWidget(maxName, 0, 0);
    lutGrid->addWidget(m_lutMaxSlider, 0, 1);
    lutGrid->addWidget(m_lutMaxLabel, 0, 2);
    lutGrid->addWidget(minName, 1, 0);
    lutGrid->addWidget(m_lutMinSlider, 1, 1);
    lutGrid->addWidget(m_lutMinLabel, 1, 2);
    lutGrid->setColumnStretch(1, 1);
    colorContent->addLayout(lutGrid);

    root->addWidget(m_leftPanel);
    root->addWidget(center, 1);
    root->addWidget(m_colorPanel);
    setCentralWidget(central);

    auto *fileMenu = menuBar()->addMenu(QStringLiteral("文件"));
    auto *openAction = fileMenu->addAction(QStringLiteral("打开..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);

    auto *viewMenu = menuBar()->addMenu(QStringLiteral("视图"));
    auto *resetAction = viewMenu->addAction(QStringLiteral("重置视角"));
    resetAction->setShortcut(Qt::Key_Home);
    connect(resetAction, &QAction::triggered, this, &MainWindow::onResetCamera);

    m_statusLabel = new QLabel(QStringLiteral("正在加载仿真场景..."));
    statusBar()->addPermanentWidget(m_statusLabel, 1);
}

void MainWindow::onLeftPanelCollapsed(bool collapsed)
{
    m_leftPanel->setFixedWidth(collapsed ? 52 : 292);
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
    if (value >= m_lutMaxSlider->value()) {
        m_lutMinSlider->blockSignals(true);
        m_lutMinSlider->setValue(m_lutMaxSlider->value() - 1);
        m_lutMinSlider->blockSignals(false);
        value = m_lutMinSlider->value();
    }
    m_lutMinLabel->setText(QString::number(value));
    m_colorBar->setRange(value, m_lutMaxSlider->value());
    m_viewer->setLutRange(value, m_lutMaxSlider->value());
}

void MainWindow::onLutMaxChanged(int value)
{
    if (value <= m_lutMinSlider->value()) {
        m_lutMaxSlider->blockSignals(true);
        m_lutMaxSlider->setValue(m_lutMinSlider->value() + 1);
        m_lutMaxSlider->blockSignals(false);
        value = m_lutMaxSlider->value();
    }
    m_lutMaxLabel->setText(QString::number(value));
    m_colorBar->setRange(m_lutMinSlider->value(), value);
    m_viewer->setLutRange(m_lutMinSlider->value(), value);
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
