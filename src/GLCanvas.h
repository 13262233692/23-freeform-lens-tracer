#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QElapsedTimer>
#include <memory>
#include "Renderer/Camera.h"
#include "Renderer/GLRenderer.h"
#include "NURBS/NURBSSurface.h"
#include "NURBS/NURBSTessellator.h"
#include "RayTracer/RayTracerEngine.h"

class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    explicit GLCanvas(QWidget* parent = nullptr);
    ~GLCanvas();

    GLRenderer* renderer() { return m_renderer.get(); }
    Camera* camera() { return &m_camera; }
    RayTracerEngine* rayTracer() { return &m_rayTracer; }
    NURBSSurface* surface() { return m_surface.get(); }

    void setSurface(std::unique_ptr<NURBSSurface> surface);
    void refreshMesh();
    void refreshRays();

signals:
    void statusMessage(const QString& msg);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void onTracingProgress(int percent);
    void onTracingFinished();

    std::unique_ptr<GLRenderer> m_renderer;
    Camera m_camera;
    NURBSTessellator m_tessellator;
    RayTracerEngine m_rayTracer;
    std::unique_ptr<NURBSSurface> m_surface;

    QPoint m_lastMousePos;
    bool m_leftButtonPressed = false;
    bool m_rightButtonPressed = false;
    bool m_middleButtonPressed = false;

    QElapsedTimer m_frameTimer;
};
