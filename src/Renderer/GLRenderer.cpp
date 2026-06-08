#include "GLRenderer.h"
#include <QFile>
#include <QTextStream>

GLRenderer::GLRenderer() = default;

GLRenderer::~GLRenderer()
{
    cleanup();
}

bool GLRenderer::createShaderProgram(QOpenGLShaderProgram*& program,
                                      const QString& vertPath,
                                      const QString& fragPath)
{
    program = new QOpenGLShaderProgram();

    QFile vf(vertPath);
    QFile ff(fragPath);

    QString vertSrc, fragSrc;
    if (vf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&vf);
        vertSrc = in.readAll();
        vf.close();
    } else {
        return false;
    }

    if (ff.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&ff);
        fragSrc = in.readAll();
        ff.close();
    } else {
        return false;
    }

    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc)) return false;
    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc)) return false;
    if (!program->link()) return false;

    return true;
}

bool GLRenderer::initialize()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);

    m_surfaceProgram = new QOpenGLShaderProgram();
    if (!m_surfaceProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/surface.vert") ||
        !m_surfaceProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/surface.frag") ||
        !m_surfaceProgram->link()) {
        return false;
    }

    m_rayProgram = new QOpenGLShaderProgram();
    if (!m_rayProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/ray.vert") ||
        !m_rayProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/ray.frag") ||
        !m_rayProgram->link()) {
        return false;
    }

    m_lightProgram = new QOpenGLShaderProgram();
    if (!m_lightProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/light.vert") ||
        !m_lightProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/light.frag") ||
        !m_lightProgram->link()) {
        return false;
    }

    m_surfaceVAO.create();
    m_surfaceVBO.create();
    m_surfaceNBO.create();
    m_surfaceIBO.create();

    m_rayVAO.create();
    m_rayVBO.create();

    m_lightVAO.create();
    m_lightVBO.create();

    m_initialized = true;
    return true;
}

void GLRenderer::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

void GLRenderer::uploadSurfaceMesh(const MeshData& mesh)
{
    if (!m_initialized || mesh.isEmpty()) return;

    m_surfaceVAO.bind();

    m_surfaceVBO.bind();
    m_surfaceVBO.allocate(mesh.vertices.data(), mesh.vertices.size() * sizeof(float));

    m_surfaceProgram->enableAttributeArray(0);
    m_surfaceProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    m_surfaceNBO.bind();
    m_surfaceNBO.allocate(mesh.normals.data(), mesh.normals.size() * sizeof(float));

    m_surfaceProgram->enableAttributeArray(1);
    m_surfaceProgram->setAttributeBuffer(1, GL_FLOAT, 0, 3);

    m_surfaceIBO.bind();
    m_surfaceIBO.allocate(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int));

    m_surfaceIndexCount = mesh.indices.size();

    m_surfaceVAO.release();
}

void GLRenderer::uploadRayPaths(const std::vector<RayPath>& paths)
{
    if (!m_initialized) return;

    std::vector<float> allRayVerts;
    allRayVerts.reserve(paths.size() * 20);

    for (const auto& path : paths) {
        for (float p : path.points) {
            allRayVerts.push_back(p);
        }
    }

    m_rayVAO.bind();
    m_rayVBO.bind();
    m_rayVBO.allocate(allRayVerts.data(), allRayVerts.size() * sizeof(float));

    m_rayProgram->enableAttributeArray(0);
    m_rayProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    m_rayVertexCount = allRayVerts.size() / 3;

    m_rayVAO.release();
}

void GLRenderer::uploadLightPosition(const QVector3D& pos)
{
    m_lightPos = pos;

    if (!m_initialized) return;

    float pt[] = { pos.x(), pos.y(), pos.z() };

    m_lightVAO.bind();
    m_lightVBO.bind();
    m_lightVBO.allocate(pt, 3 * sizeof(float));

    m_lightProgram->enableAttributeArray(0);
    m_lightProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    m_lightVAO.release();
}

void GLRenderer::render(const QMatrix4x4& view, const QMatrix4x4& projection, const QVector3D& cameraPos)
{
    if (!m_initialized) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSurface(view, projection, cameraPos);

    if (m_showRays)
        renderRays(view, projection);

    if (m_showLightSource)
        renderLightSource(view, projection);
}

void GLRenderer::renderSurface(const QMatrix4x4& view, const QMatrix4x4& projection, const QVector3D& cameraPos)
{
    if (m_surfaceIndexCount == 0) return;

    m_surfaceProgram->bind();

    QMatrix4x4 model;
    model.setToIdentity();

    m_surfaceProgram->setUniformValue("uModel", model);
    m_surfaceProgram->setUniformValue("uView", view);
    m_surfaceProgram->setUniformValue("uProjection", projection);

    QMatrix3x3 normalMatrix = model.normalMatrix();
    m_surfaceProgram->setUniformValue("uNormalMatrix", normalMatrix);

    m_surfaceProgram->setUniformValue("uLightPos", m_lightPos);
    m_surfaceProgram->setUniformValue("uViewPos", cameraPos);
    m_surfaceProgram->setUniformValue("uSurfaceColor", m_surfaceColor);
    m_surfaceProgram->setUniformValue("uAlpha", m_surfaceAlpha);

    m_surfaceVAO.bind();

    if (m_showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        m_surfaceProgram->setUniformValue("uAlpha", 1.0f);
    }

    glDrawElements(GL_TRIANGLES, m_surfaceIndexCount, GL_UNSIGNED_INT, nullptr);

    if (m_showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    m_surfaceVAO.release();
    m_surfaceProgram->release();
}

void GLRenderer::renderRays(const QMatrix4x4& view, const QMatrix4x4& projection)
{
    if (m_rayVertexCount == 0) return;

    m_rayProgram->bind();
    m_rayProgram->setUniformValue("uView", view);
    m_rayProgram->setUniformValue("uProjection", projection);
    m_rayProgram->setUniformValue("uRayColor", m_rayColor);
    m_rayProgram->setUniformValue("uRayAlpha", 0.85f);

    glLineWidth(1.5f);

    m_rayVAO.bind();
    glDrawArrays(GL_LINES, 0, m_rayVertexCount);
    m_rayVAO.release();

    m_rayProgram->release();
}

void GLRenderer::renderLightSource(const QMatrix4x4& view, const QMatrix4x4& projection)
{
    m_lightProgram->bind();
    m_lightProgram->setUniformValue("uView", view);
    m_lightProgram->setUniformValue("uProjection", projection);
    m_lightProgram->setUniformValue("uLightColor", QVector3D(1.0f, 0.95f, 0.8f));

    m_lightVAO.bind();
    glDrawArrays(GL_POINTS, 0, 1);
    m_lightVAO.release();

    m_lightProgram->release();
}

void GLRenderer::setSurfaceColor(const QVector3D& color) { m_surfaceColor = color; }
void GLRenderer::setSurfaceAlpha(float alpha) { m_surfaceAlpha = alpha; }
void GLRenderer::setRayColor(const QVector3D& color) { m_rayColor = color; }
void GLRenderer::setShowWireframe(bool show) { m_showWireframe = show; }
void GLRenderer::setShowRays(bool show) { m_showRays = show; }
void GLRenderer::setShowLightSource(bool show) { m_showLightSource = show; }

void GLRenderer::cleanup()
{
    if (!m_initialized) return;

    m_surfaceVAO.destroy();
    m_surfaceVBO.destroy();
    m_surfaceNBO.destroy();
    m_surfaceIBO.destroy();
    m_rayVAO.destroy();
    m_rayVBO.destroy();
    m_lightVAO.destroy();
    m_lightVBO.destroy();

    delete m_surfaceProgram; m_surfaceProgram = nullptr;
    delete m_rayProgram; m_rayProgram = nullptr;
    delete m_lightProgram; m_lightProgram = nullptr;

    m_initialized = false;
}
