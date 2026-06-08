#pragma once

#include <QVector3D>
#include <QObject>
#include <vector>
#include <atomic>
#include <future>
#include "RayTracer/OpticalMath.h"
#include "Renderer/GLRenderer.h"

class NURBSSurface;

struct LightSourceConfig
{
    QVector3D position{0.0f, 0.0f, 5.0f};
    QVector3D direction{0.0f, 0.0f, -1.0f};
    float halfAngle = 0.35f;
    int rayCount = 10000;
    float nMedium = 1.4925f;
    float nEnvironment = 1.0f;
    int maxBounces = 32;
    float energyThreshold = 0.01f;
    float selfIntersectEpsilon = 0.01f;
};

class RayTracerEngine : public QObject
{
    Q_OBJECT

public:
    explicit RayTracerEngine(QObject* parent = nullptr);
    ~RayTracerEngine();

    void setSurface(const NURBSSurface* surface);
    void setLightSource(const LightSourceConfig& config);
    LightSourceConfig lightSourceConfig() const { return m_lightConfig; }
    void setTessellationResolution(int resU, int resV);

    void startTracing();
    void cancelTracing();
    bool isRunning() const;

    std::vector<RayPath> rayPaths() const;
    int progress() const;

signals:
    void tracingProgress(int percent);
    void tracingFinished();

private:
    void traceRays();
    std::vector<RayPath> traceRaysThread(int startIdx, int count);
    OpticalMath::HitRecord intersectRaySurface(const OpticalMath::Ray& ray) const;
    OpticalMath::HitRecord intersectRayTriangle(const OpticalMath::Ray& ray,
                                                 const QVector3D& v0,
                                                 const QVector3D& v1,
                                                 const QVector3D& v2) const;

    void buildAccelerationStructure();

    const NURBSSurface* m_surface = nullptr;
    LightSourceConfig m_lightConfig;
    int m_tessResU = 64;
    int m_tessResV = 64;

    std::vector<QVector3D> m_meshVertices;
    std::vector<QVector3D> m_meshNormals;
    std::vector<unsigned int> m_meshIndices;

    std::vector<RayPath> m_rayPaths;
    std::atomic<bool> m_cancelFlag{false};
    std::atomic<bool> m_running{false};
    std::atomic<int> m_progress{0};
    int m_totalRays = 0;

    int m_threadCount = 0;
};
