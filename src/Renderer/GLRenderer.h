#pragma once

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector3D>
#include <memory>
#include "NURBS/NURBSTessellator.h"

struct RayPath
{
    std::vector<float> points;
    std::vector<float> intensities;
    QVector3D color;
    float maxIntensity = 0.0f;
    int segmentCount() const { return static_cast<int>(points.size()) / 3 - 1; }
    void addPoint(const QVector3D& p, float intensity = 1.0f) {
        points.push_back(p.x());
        points.push_back(p.y());
        points.push_back(p.z());
        intensities.push_back(intensity);
        if (intensity > maxIntensity) maxIntensity = intensity;
    }
};

class GLRenderer : protected QOpenGLFunctions_4_5_Core
{
public:
    GLRenderer();
    ~GLRenderer();

    bool initialize();
    void resize(int width, int height);
    void render(const QMatrix4x4& view, const QMatrix4x4& projection, const QVector3D& cameraPos);

    void uploadSurfaceMesh(const MeshData& mesh);
    void uploadRayPaths(const std::vector<RayPath>& paths);
    void uploadLightPosition(const QVector3D& pos);

    void setSurfaceColor(const QVector3D& color);
    void setSurfaceAlpha(float alpha);
    void setRayColor(const QVector3D& color);
    void setShowWireframe(bool show);
    void setShowRays(bool show);
    void setShowLightSource(bool show);

    void cleanup();

private:
    bool createShaderProgram(QOpenGLShaderProgram*& program,
                             const QString& vertPath,
                             const QString& fragPath);

    void renderSurface(const QMatrix4x4& view, const QMatrix4x4& projection, const QVector3D& cameraPos);
    void renderRays(const QMatrix4x4& view, const QMatrix4x4& projection);
    void renderLightSource(const QMatrix4x4& view, const QMatrix4x4& projection);

    QOpenGLShaderProgram* m_surfaceProgram = nullptr;
    QOpenGLShaderProgram* m_rayProgram = nullptr;
    QOpenGLShaderProgram* m_lightProgram = nullptr;

    QOpenGLVertexArrayObject m_surfaceVAO;
    QOpenGLBuffer m_surfaceVBO{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_surfaceNBO{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_surfaceIBO{QOpenGLBuffer::IndexBuffer};
    int m_surfaceIndexCount = 0;

    QOpenGLVertexArrayObject m_rayVAO;
    QOpenGLBuffer m_rayVBO{QOpenGLBuffer::VertexBuffer};
    int m_rayVertexCount = 0;

    QOpenGLVertexArrayObject m_lightVAO;
    QOpenGLBuffer m_lightVBO{QOpenGLBuffer::VertexBuffer};

    QVector3D m_surfaceColor{0.3f, 0.6f, 0.9f};
    float m_surfaceAlpha = 0.6f;
    QVector3D m_rayColor{1.0f, 0.85f, 0.2f};
    QVector3D m_lightPos{0.0f, 0.0f, 5.0f};

    bool m_showWireframe = false;
    bool m_showRays = true;
    bool m_showLightSource = true;
    bool m_initialized = false;

    int m_width = 800;
    int m_height = 600;
};
