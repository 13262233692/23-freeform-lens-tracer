#pragma once

#include <QMainWindow>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QGroupBox>
#include <QFormLayout>

class GLCanvas;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLensTypeChanged(int index);
    void onRunTrace();
    void onCancelTrace();
    void onSurfaceAlphaChanged(int value);
    void onRayCountChanged(int value);
    void onLensRadiusChanged(double value);
    void onConicConstantChanged(double value);
    void onRefractiveIndexChanged(double value);
    void onToggleWireframe(bool checked);
    void onToggleRays(bool checked);
    void onToggleLightSource(bool checked);
    void onLightHalfAngleChanged(double value);
    void onFreeformAmplitudeChanged(double value);

private:
    void createActions();
    void createMenuBar();
    void createToolBar();
    QDockWidget* createLensParametersPanel();
    QDockWidget* createLightSourcePanel();
    QDockWidget* createRenderSettingsPanel();
    void createStatusBar();
    void rebuildSurface();

    GLCanvas* m_canvas = nullptr;

    QComboBox* m_lensTypeCombo = nullptr;
    QDoubleSpinBox* m_lensRadiusSpin = nullptr;
    QDoubleSpinBox* m_conicConstantSpin = nullptr;
    QDoubleSpinBox* m_freeformAmplitudeSpin = nullptr;
    QDoubleSpinBox* m_apertureRadiusSpin = nullptr;

    QDoubleSpinBox* m_lightPosXSpin = nullptr;
    QDoubleSpinBox* m_lightPosYSpin = nullptr;
    QDoubleSpinBox* m_lightPosZSpin = nullptr;
    QDoubleSpinBox* m_refractiveIndexSpin = nullptr;
    QDoubleSpinBox* m_lightHalfAngleSpin = nullptr;
    QSpinBox* m_rayCountSpin = nullptr;
    QSpinBox* m_maxBouncesSpin = nullptr;

    QSlider* m_surfaceAlphaSlider = nullptr;
    QCheckBox* m_wireframeCheck = nullptr;
    QCheckBox* m_raysCheck = nullptr;
    QCheckBox* m_lightCheck = nullptr;

    QPushButton* m_runTraceBtn = nullptr;
    QPushButton* m_cancelTraceBtn = nullptr;
    QProgressBar* m_traceProgress = nullptr;
};
