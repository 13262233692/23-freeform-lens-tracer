#include "MainWindow.h"
#include "GLCanvas.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Freeform Lens Tracer — Automotive Adaptive Headlamp Design Platform"));
    resize(1400, 900);

    m_canvas = new GLCanvas(this);
    setCentralWidget(m_canvas);

    createActions();
    createMenuBar();
    createToolBar();

    addDockWidget(Qt::LeftDockWidgetArea, createLensParametersPanel());
    addDockWidget(Qt::RightDockWidgetArea, createLightSourcePanel());
    addDockWidget(Qt::RightDockWidgetArea, createRenderSettingsPanel());

    createStatusBar();

    connect(m_canvas, &GLCanvas::statusMessage, this, [this](const QString& msg) {
        statusBar()->showMessage(msg);
    });

    auto defaultSurface = std::make_unique<NURBSSurface>(
        NURBSSurface::createAsphericLens(25.0f, -0.5f, 12.0f));
    m_canvas->setSurface(std::move(defaultSurface));

    QTimer::singleShot(100, this, [this]() {
        m_canvas->refreshMesh();
        m_canvas->renderer()->uploadLightPosition(
            m_canvas->rayTracer()->lightSourceConfig().position);
        statusBar()->showMessage(tr("Ready — Aspheric lens loaded"));
    });
}

MainWindow::~MainWindow() = default;

void MainWindow::createActions()
{
}

void MainWindow::createMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Reset View"), this, [this]() {
        m_canvas->camera()->setPosition(QVector3D(5.0f, 3.0f, 8.0f));
        m_canvas->camera()->setTarget(QVector3D(0, 0, 0));
        m_canvas->update();
    });
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

    QMenu* simMenu = menuBar()->addMenu(tr("&Simulation"));
    simMenu->addAction(tr("&Run Ray Trace"), this, &MainWindow::onRunTrace);
    simMenu->addAction(tr("&Cancel Trace"), this, &MainWindow::onCancelTrace);

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, [this]() {
        QMessageBox::about(this, tr("About Freeform Lens Tracer"),
            tr("Professional optical lens design platform for automotive adaptive headlamp systems.\n\n"
               "Features:\n"
               "• NURBS freeform surface modeling\n"
               "• Monte Carlo ray tracing with Snell's law\n"
               "• Multi-threaded optical simulation\n"
               "• Real-time OpenGL visualization"));
    });
}

void MainWindow::createToolBar()
{
    QToolBar* toolbar = addToolBar(tr("Main Toolbar"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    m_runTraceBtn = new QPushButton(tr("▶ Run Trace"));
    m_runTraceBtn->setStyleSheet("QPushButton { background-color: #2d5aa0; color: white; padding: 6px 16px; border-radius: 4px; font-weight: bold; }");
    toolbar->addWidget(m_runTraceBtn);
    connect(m_runTraceBtn, &QPushButton::clicked, this, &MainWindow::onRunTrace);

    m_cancelTraceBtn = new QPushButton(tr("✕ Cancel"));
    m_cancelTraceBtn->setStyleSheet("QPushButton { background-color: #a02d2d; color: white; padding: 6px 16px; border-radius: 4px; font-weight: bold; }");
    m_cancelTraceBtn->setEnabled(false);
    toolbar->addWidget(m_cancelTraceBtn);
    connect(m_cancelTraceBtn, &QPushButton::clicked, this, &MainWindow::onCancelTrace);

    toolbar->addSeparator();

    m_wireframeCheck = new QCheckBox(tr("Wireframe"));
    m_wireframeCheck->setStyleSheet("QCheckBox { color: #ddd; padding: 4px; }");
    toolbar->addWidget(m_wireframeCheck);
    connect(m_wireframeCheck, &QCheckBox::toggled, this, &MainWindow::onToggleWireframe);

    m_raysCheck = new QCheckBox(tr("Show Rays"));
    m_raysCheck->setChecked(true);
    m_raysCheck->setStyleSheet("QCheckBox { color: #ddd; padding: 4px; }");
    toolbar->addWidget(m_raysCheck);
    connect(m_raysCheck, &QCheckBox::toggled, this, &MainWindow::onToggleRays);

    m_lightCheck = new QCheckBox(tr("Light Source"));
    m_lightCheck->setChecked(true);
    m_lightCheck->setStyleSheet("QCheckBox { color: #ddd; padding: 4px; }");
    toolbar->addWidget(m_lightCheck);
    connect(m_lightCheck, &QCheckBox::toggled, this, &MainWindow::onToggleLightSource);
}

QDockWidget* MainWindow::createLensParametersPanel()
{
    QDockWidget* dock = new QDockWidget(tr("Lens Parameters"), this);
    dock->setMinimumWidth(280);

    QWidget* content = new QWidget();
    QFormLayout* layout = new QFormLayout(content);
    layout->setSpacing(8);
    layout->setContentsMargins(12, 12, 12, 12);

    m_lensTypeCombo = new QComboBox();
    m_lensTypeCombo->addItems({tr("Aspheric Lens"), tr("Freeform Lens")});
    connect(m_lensTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLensTypeChanged);
    layout->addRow(tr("Type:"), m_lensTypeCombo);

    m_lensRadiusSpin = new QDoubleSpinBox();
    m_lensRadiusSpin->setRange(5.0, 100.0);
    m_lensRadiusSpin->setValue(25.0);
    m_lensRadiusSpin->setSingleStep(1.0);
    m_lensRadiusSpin->setSuffix(" mm");
    connect(m_lensRadiusSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onLensRadiusChanged);
    layout->addRow(tr("Base Radius:"), m_lensRadiusSpin);

    m_conicConstantSpin = new QDoubleSpinBox();
    m_conicConstantSpin->setRange(-10.0, 10.0);
    m_conicConstantSpin->setValue(-0.5);
    m_conicConstantSpin->setSingleStep(0.1);
    connect(m_conicConstantSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onConicConstantChanged);
    layout->addRow(tr("Conic Constant:"), m_conicConstantSpin);

    m_freeformAmplitudeSpin = new QDoubleSpinBox();
    m_freeformAmplitudeSpin->setRange(0.0, 5.0);
    m_freeformAmplitudeSpin->setValue(1.5);
    m_freeformAmplitudeSpin->setSingleStep(0.1);
    m_freeformAmplitudeSpin->setSuffix(" mm");
    m_freeformAmplitudeSpin->setEnabled(false);
    connect(m_freeformAmplitudeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onFreeformAmplitudeChanged);
    layout->addRow(tr("Freeform Amp.:"), m_freeformAmplitudeSpin);

    m_apertureRadiusSpin = new QDoubleSpinBox();
    m_apertureRadiusSpin->setRange(2.0, 30.0);
    m_apertureRadiusSpin->setValue(12.0);
    m_apertureRadiusSpin->setSingleStep(0.5);
    m_apertureRadiusSpin->setSuffix(" mm");
    layout->addRow(tr("Aperture Radius:"), m_apertureRadiusSpin);

    dock->setWidget(content);
    return dock;
}

QDockWidget* MainWindow::createLightSourcePanel()
{
    QDockWidget* dock = new QDockWidget(tr("LED Light Source"), this);
    dock->setMinimumWidth(280);

    QWidget* content = new QWidget();
    QFormLayout* layout = new QFormLayout(content);
    layout->setSpacing(8);
    layout->setContentsMargins(12, 12, 12, 12);

    m_lightPosXSpin = new QDoubleSpinBox();
    m_lightPosXSpin->setRange(-50.0, 50.0);
    m_lightPosXSpin->setValue(0.0);
    m_lightPosXSpin->setSingleStep(1.0);
    layout->addRow(tr("Position X:"), m_lightPosXSpin);

    m_lightPosYSpin = new QDoubleSpinBox();
    m_lightPosYSpin->setRange(-50.0, 50.0);
    m_lightPosYSpin->setValue(0.0);
    m_lightPosYSpin->setSingleStep(1.0);
    layout->addRow(tr("Position Y:"), m_lightPosYSpin);

    m_lightPosZSpin = new QDoubleSpinBox();
    m_lightPosZSpin->setRange(-50.0, 50.0);
    m_lightPosZSpin->setValue(5.0);
    m_lightPosZSpin->setSingleStep(1.0);
    layout->addRow(tr("Position Z:"), m_lightPosZSpin);

    m_refractiveIndexSpin = new QDoubleSpinBox();
    m_refractiveIndexSpin->setRange(1.0, 3.0);
    m_refractiveIndexSpin->setValue(1.4925);
    m_refractiveIndexSpin->setSingleStep(0.01);
    m_refractiveIndexSpin->setDecimals(4);
    connect(m_refractiveIndexSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onRefractiveIndexChanged);
    layout->addRow(tr("Refractive Index:"), m_refractiveIndexSpin);

    m_lightHalfAngleSpin = new QDoubleSpinBox();
    m_lightHalfAngleSpin->setRange(5.0, 90.0);
    m_lightHalfAngleSpin->setValue(20.0);
    m_lightHalfAngleSpin->setSingleStep(1.0);
    m_lightHalfAngleSpin->setSuffix("°");
    connect(m_lightHalfAngleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onLightHalfAngleChanged);
    layout->addRow(tr("Half Angle:"), m_lightHalfAngleSpin);

    m_rayCountSpin = new QSpinBox();
    m_rayCountSpin->setRange(100, 100000);
    m_rayCountSpin->setValue(10000);
    m_rayCountSpin->setSingleStep(1000);
    connect(m_rayCountSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onRayCountChanged);
    layout->addRow(tr("Ray Count:"), m_rayCountSpin);

    m_maxBouncesSpin = new QSpinBox();
    m_maxBouncesSpin->setRange(1, 20);
    m_maxBouncesSpin->setValue(5);
    layout->addRow(tr("Max Bounces:"), m_maxBouncesSpin);

    dock->setWidget(content);
    return dock;
}

QDockWidget* MainWindow::createRenderSettingsPanel()
{
    QDockWidget* dock = new QDockWidget(tr("Render Settings"), this);
    dock->setMinimumWidth(280);

    QWidget* content = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setSpacing(8);
    layout->setContentsMargins(12, 12, 12, 12);

    QGroupBox* surfaceGroup = new QGroupBox(tr("Surface"));
    QFormLayout* surfaceLayout = new QFormLayout(surfaceGroup);

    m_surfaceAlphaSlider = new QSlider(Qt::Horizontal);
    m_surfaceAlphaSlider->setRange(10, 100);
    m_surfaceAlphaSlider->setValue(60);
    connect(m_surfaceAlphaSlider, &QSlider::valueChanged, this, &MainWindow::onSurfaceAlphaChanged);
    surfaceLayout->addRow(tr("Opacity:"), m_surfaceAlphaSlider);

    layout->addWidget(surfaceGroup);

    QGroupBox* progressGroup = new QGroupBox(tr("Tracing Progress"));
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);

    m_traceProgress = new QProgressBar();
    m_traceProgress->setRange(0, 100);
    m_traceProgress->setValue(0);
    progressLayout->addWidget(m_traceProgress);

    layout->addWidget(progressGroup);
    layout->addStretch();

    dock->setWidget(content);
    return dock;
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::onLensTypeChanged(int index)
{
    m_conicConstantSpin->setEnabled(index == 0);
    m_freeformAmplitudeSpin->setEnabled(index == 1);
    rebuildSurface();
}

void MainWindow::rebuildSurface()
{
    std::unique_ptr<NURBSSurface> surface;

    float radius = m_lensRadiusSpin->value();
    float aperture = m_apertureRadiusSpin->value();

    if (m_lensTypeCombo->currentIndex() == 0) {
        float conic = m_conicConstantSpin->value();
        surface = std::make_unique<NURBSSurface>(
            NURBSSurface::createAsphericLens(radius, conic, aperture));
    } else {
        float amp = m_freeformAmplitudeSpin->value();
        surface = std::make_unique<NURBSSurface>(
            NURBSSurface::createFreeformLens(radius, amp, aperture));
    }

    m_canvas->setSurface(std::move(surface));
    m_canvas->refreshMesh();
    m_canvas->update();
}

void MainWindow::onRunTrace()
{
    if (m_canvas->rayTracer()->isRunning()) return;

    LightSourceConfig config;
    config.position = QVector3D(
        m_lightPosXSpin->value(),
        m_lightPosYSpin->value(),
        m_lightPosZSpin->value());
    config.nMedium = m_refractiveIndexSpin->value();
    config.rayCount = m_rayCountSpin->value();
    config.maxBounces = m_maxBouncesSpin->value();
    config.halfAngle = m_lightHalfAngleSpin->value() * 3.14159265f / 180.0f;

    m_canvas->rayTracer()->setLightSource(config);
    m_canvas->renderer()->uploadLightPosition(config.position);

    m_runTraceBtn->setEnabled(false);
    m_cancelTraceBtn->setEnabled(true);
    m_traceProgress->setValue(0);

    connect(m_canvas->rayTracer(), &RayTracerEngine::tracingProgress,
            this, [this](int value) { m_traceProgress->setValue(value); });

    connect(m_canvas->rayTracer(), &RayTracerEngine::tracingFinished, this, [this]() {
        m_runTraceBtn->setEnabled(true);
        m_cancelTraceBtn->setEnabled(false);
    });

    m_canvas->rayTracer()->startTracing();
}

void MainWindow::onCancelTrace()
{
    m_canvas->rayTracer()->cancelTracing();
    m_runTraceBtn->setEnabled(true);
    m_cancelTraceBtn->setEnabled(false);
}

void MainWindow::onSurfaceAlphaChanged(int value)
{
    if (m_canvas->renderer())
        m_canvas->renderer()->setSurfaceAlpha(value / 100.0f);
    m_canvas->update();
}

void MainWindow::onRayCountChanged(int value)
{
    Q_UNUSED(value);
}

void MainWindow::onLensRadiusChanged(double)
{
    rebuildSurface();
}

void MainWindow::onConicConstantChanged(double)
{
    rebuildSurface();
}

void MainWindow::onRefractiveIndexChanged(double)
{
}

void MainWindow::onToggleWireframe(bool checked)
{
    if (m_canvas->renderer())
        m_canvas->renderer()->setShowWireframe(checked);
    m_canvas->update();
}

void MainWindow::onToggleRays(bool checked)
{
    if (m_canvas->renderer())
        m_canvas->renderer()->setShowRays(checked);
    m_canvas->update();
}

void MainWindow::onToggleLightSource(bool checked)
{
    if (m_canvas->renderer())
        m_canvas->renderer()->setShowLightSource(checked);
    m_canvas->update();
}

void MainWindow::onLightHalfAngleChanged(double)
{
}

void MainWindow::onFreeformAmplitudeChanged(double)
{
    rebuildSurface();
}
