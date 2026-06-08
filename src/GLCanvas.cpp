#include "GLCanvas.h"
#include <QMouseEvent>
#include <QWheelEvent>

GLCanvas::GLCanvas(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(800, 600);

    connect(&m_rayTracer, &RayTracerEngine::tracingProgress,
            this, &GLCanvas::onTracingProgress);
    connect(&m_rayTracer, &RayTracerEngine::tracingFinished,
            this, &GLCanvas::onTracingFinished);
}

GLCanvas::~GLCanvas()
{
    makeCurrent();
    if (m_renderer) m_renderer->cleanup();
    doneCurrent();
}

void GLCanvas::initializeGL()
{
    initializeOpenGLFunctions();

    m_renderer = std::make_unique<GLRenderer>();
    if (!m_renderer->initialize()) {
        emit statusMessage(tr("Failed to initialize OpenGL renderer"));
        return;
    }

    if (m_surface) {
        refreshMesh();
    }
}

void GLCanvas::resizeGL(int w, int h)
{
    if (m_renderer) m_renderer->resize(w, h);
}

void GLCanvas::paintGL()
{
    if (!m_renderer) return;

    QMatrix4x4 view = m_camera.viewMatrix();
    float aspect = float(width()) / std::max(1, height());
    QMatrix4x4 proj = m_camera.projectionMatrix(aspect);

    m_renderer->render(view, proj, m_camera.position());
}

void GLCanvas::setSurface(std::unique_ptr<NURBSSurface> surface)
{
    m_surface = std::move(surface);
    m_rayTracer.setSurface(m_surface.get());
}

void GLCanvas::refreshMesh()
{
    if (!m_surface || !m_renderer) return;

    MeshData mesh = m_tessellator.tessellate(*m_surface);
    m_renderer->uploadSurfaceMesh(mesh);
    update();
}

void GLCanvas::refreshRays()
{
    if (!m_renderer) return;

    auto paths = m_rayTracer.rayPaths();
    m_renderer->uploadRayPaths(paths);
    m_renderer->uploadLightPosition(m_rayTracer.lightSourceConfig().position);
    update();
}

void GLCanvas::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePos = event->pos();
    m_leftButtonPressed = (event->button() == Qt::LeftButton);
    m_rightButtonPressed = (event->button() == Qt::RightButton);
    m_middleButtonPressed = (event->button() == Qt::MiddleButton);
}

void GLCanvas::mouseMoveEvent(QMouseEvent* event)
{
    QPointF delta = event->pos() - m_lastMousePos;

    if (m_leftButtonPressed) {
        m_camera.orbit(delta);
    } else if (m_rightButtonPressed || m_middleButtonPressed) {
        m_camera.pan(delta);
    }

    m_lastMousePos = event->pos();
    update();
}

void GLCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    m_leftButtonPressed = false;
    m_rightButtonPressed = false;
    m_middleButtonPressed = false;
}

void GLCanvas::wheelEvent(QWheelEvent* event)
{
    float delta = event->angleDelta().y() / 120.0f;
    m_camera.zoom(delta);
    update();
}

void GLCanvas::onTracingProgress(int percent)
{
    emit statusMessage(tr("Ray tracing: %1%").arg(percent));
}

void GLCanvas::onTracingFinished()
{
    refreshRays();
    emit statusMessage(tr("Ray tracing complete: %1 rays traced").arg(m_rayTracer.rayPaths().size()));
}
