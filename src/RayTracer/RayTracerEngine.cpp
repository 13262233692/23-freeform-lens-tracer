#include "RayTracerEngine.h"
#include "NURBS/NURBSSurface.h"
#include "NURBS/NURBSTessellator.h"
#include <QVector3D>
#include <thread>
#include <algorithm>

RayTracerEngine::RayTracerEngine(QObject* parent)
    : QObject(parent)
{
    m_threadCount = std::max(1, (int)std::thread::hardware_concurrency());
}

RayTracerEngine::~RayTracerEngine()
{
    cancelTracing();
}

void RayTracerEngine::setSurface(const NURBSSurface* surface)
{
    m_surface = surface;
}

void RayTracerEngine::setLightSource(const LightSourceConfig& config)
{
    m_lightConfig = config;
}

void RayTracerEngine::setTessellationResolution(int resU, int resV)
{
    m_tessResU = resU;
    m_tessResV = resV;
}

void RayTracerEngine::buildAccelerationStructure()
{
    m_meshVertices.clear();
    m_meshNormals.clear();
    m_meshIndices.clear();

    if (!m_surface) return;

    NURBSTessellator tess;
    tess.setResolution(m_tessResU, m_tessResV);
    MeshData mesh = tess.tessellate(*m_surface);

    int vCount = mesh.vertexCount();
    m_meshVertices.resize(vCount);
    m_meshNormals.resize(vCount);

    for (int i = 0; i < vCount; ++i) {
        m_meshVertices[i] = QVector3D(
            mesh.vertices[i * 3],
            mesh.vertices[i * 3 + 1],
            mesh.vertices[i * 3 + 2]);
        m_meshNormals[i] = QVector3D(
            mesh.normals[i * 3],
            mesh.normals[i * 3 + 1],
            mesh.normals[i * 3 + 2]);
    }

    m_meshIndices = mesh.indices;
}

OpticalMath::HitRecord RayTracerEngine::intersectRayTriangle(const OpticalMath::Ray& ray,
                                                              const QVector3D& v0,
                                                              const QVector3D& v1,
                                                              const QVector3D& v2) const
{
    OpticalMath::HitRecord rec;

    QVector3D edge1 = v1 - v0;
    QVector3D edge2 = v2 - v0;
    QVector3D h = QVector3D::crossProduct(ray.direction, edge2);
    float a = QVector3D::dotProduct(edge1, h);

    if (std::abs(a) < 1e-8f) return rec;

    float f = 1.0f / a;
    QVector3D s = ray.origin - v0;
    float u = f * QVector3D::dotProduct(s, h);

    if (u < 0.0f || u > 1.0f) return rec;

    QVector3D q = QVector3D::crossProduct(s, edge1);
    float v = f * QVector3D::dotProduct(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f) return rec;

    float t = f * QVector3D::dotProduct(edge2, q);

    if (t > 1e-4f) {
        rec.hit = true;
        rec.t = t;
        rec.point = ray.pointAt(t);
        rec.u = u;
        rec.v = v;
    }

    return rec;
}

OpticalMath::HitRecord RayTracerEngine::intersectRaySurface(const OpticalMath::Ray& ray) const
{
    OpticalMath::HitRecord closest;

    size_t triCount = m_meshIndices.size() / 3;
    for (size_t i = 0; i < triCount; ++i) {
        unsigned int i0 = m_meshIndices[i * 3];
        unsigned int i1 = m_meshIndices[i * 3 + 1];
        unsigned int i2 = m_meshIndices[i * 3 + 2];

        OpticalMath::HitRecord rec = intersectRayTriangle(
            ray, m_meshVertices[i0], m_meshVertices[i1], m_meshVertices[i2]);

        if (rec.hit && (!closest.hit || rec.t < closest.t)) {
            closest = rec;

            float w = 1.0f - rec.u - rec.v;
            closest.normal = (w * m_meshNormals[i0] +
                              rec.u * m_meshNormals[i1] +
                              rec.v * m_meshNormals[i2]).normalized();
        }
    }

    return closest;
}

std::vector<RayPath> RayTracerEngine::traceRaysThread(int startIdx, int count)
{
    std::vector<RayPath> localPaths;
    localPaths.reserve(count);

    float nEnv = m_lightConfig.nEnvironment;
    float nMed = m_lightConfig.nMedium;
    int maxBounces = m_lightConfig.maxBounces;
    float energyThreshold = m_lightConfig.energyThreshold;
    float epsilon = m_lightConfig.selfIntersectEpsilon;

    for (int i = 0; i < count; ++i) {
        if (m_cancelFlag.load()) break;

        int rayIdx = startIdx + i;

        QVector3D localDir = OpticalMath::randomDirectionCone(m_lightConfig.halfAngle);
        QVector3D up = std::abs(m_lightConfig.direction.z()) < 0.999f
                           ? QVector3D(0, 0, 1) : QVector3D(1, 0, 0);
        QVector3D tangent = QVector3D::crossProduct(m_lightConfig.direction, up).normalized();
        QVector3D bitangent = QVector3D::crossProduct(m_lightConfig.direction, tangent);

        QVector3D worldDir = (tangent * localDir.x() +
                              bitangent * localDir.y() +
                              m_lightConfig.direction * localDir.z()).normalized();

        OpticalMath::Ray ray(m_lightConfig.position, worldDir);

        RayPath path;
        path.color = QVector3D(1.0f, 0.85f, 0.2f);

        bool inside = false;
        float energy = 1.0f;
        QVector3D lastHitPoint = ray.origin;

        path.addPoint(ray.origin, energy);

        int bounce = 0;
        while (bounce < maxBounces && energy > energyThreshold) {
            OpticalMath::HitRecord hit = intersectRaySurface(ray);

            if (!hit.hit) {
                path.addPoint(ray.pointAt(20.0f), energy);
                break;
            }

            path.addPoint(hit.point, energy);

            QVector3D surfaceNormal = hit.normal;
            float cosI = QVector3D::dotProduct(ray.direction, surfaceNormal);

            bool hittingFromInside = (cosI > 0.0f);

            QVector3D effectiveNormal;
            if (hittingFromInside) {
                effectiveNormal = -surfaceNormal;
            } else {
                effectiveNormal = surfaceNormal;
            }

            float n1 = inside ? nMed : nEnv;
            float n2 = inside ? nEnv : nMed;

            bool isTIR = OpticalMath::totalInternalReflection(ray.direction, effectiveNormal, n1, n2);

            if (isTIR) {
                QVector3D reflectedDir = OpticalMath::reflect(ray.direction, effectiveNormal);
                ray = OpticalMath::Ray(hit.point + effectiveNormal * epsilon, reflectedDir);
                energy *= 0.98f;
            } else {
                float fresnelR = OpticalMath::fresnelReflectance(ray.direction, effectiveNormal, n1, n2);

                if (OpticalMath::randomFloat() < fresnelR) {
                    QVector3D reflectedDir = OpticalMath::reflect(ray.direction, effectiveNormal);
                    ray = OpticalMath::Ray(hit.point + effectiveNormal * epsilon, reflectedDir);
                    energy *= (1.0f - fresnelR);
                } else {
                    QVector3D refractedDir = OpticalMath::refract(ray.direction, effectiveNormal, n1, n2);
                    if (refractedDir.lengthSquared() < 1e-10f) {
                        QVector3D reflectedDir = OpticalMath::reflect(ray.direction, effectiveNormal);
                        ray = OpticalMath::Ray(hit.point + effectiveNormal * epsilon, reflectedDir);
                        energy *= 0.98f;
                    } else {
                        inside = !inside;
                        ray = OpticalMath::Ray(hit.point - effectiveNormal * epsilon, refractedDir);
                        energy *= (1.0f - fresnelR);
                    }
                }
            }

            float distToLastHit = (hit.point - lastHitPoint).length();
            if (distToLastHit < epsilon * 0.5f && bounce > 0) {
                QVector3D escaped = hit.point + ray.direction * epsilon * 10.0f;
                path.addPoint(escaped, 0.0f);
                break;
            }
            lastHitPoint = hit.point;

            bounce++;
        }

        if (bounce >= maxBounces && energy > energyThreshold) {
            path.addPoint(ray.pointAt(5.0f), energy * 0.1f);
        }

        localPaths.push_back(std::move(path));

        if (rayIdx % 100 == 0) {
            int total = m_totalRays;
            int done = m_progress.fetch_add(100) + 100;
            int pct = (total > 0) ? std::min(100, done * 100 / total) : 0;
            emit tracingProgress(pct);
        }
    }

    return localPaths;
}

void RayTracerEngine::traceRays()
{
    m_rayPaths.clear();
    m_progress.store(0);
    m_cancelFlag.store(false);
    m_totalRays = m_lightConfig.rayCount;

    buildAccelerationStructure();

    int rayCount = m_lightConfig.rayCount;
    int raysPerThread = rayCount / m_threadCount;

    std::vector<std::future<std::vector<RayPath>>> futures;
    futures.reserve(m_threadCount);

    for (int t = 0; t < m_threadCount; ++t) {
        int start = t * raysPerThread;
        int count = (t == m_threadCount - 1) ? (rayCount - start) : raysPerThread;

        futures.push_back(std::async(std::launch::async, [this, start, count]() {
            return traceRaysThread(start, count);
        }));
    }

    for (auto& f : futures) {
        auto localPaths = f.get();
        m_rayPaths.insert(m_rayPaths.end(),
                          std::make_move_iterator(localPaths.begin()),
                          std::make_move_iterator(localPaths.end()));
    }

    m_progress.store(m_totalRays);
    emit tracingProgress(100);
    emit tracingFinished();
}

void RayTracerEngine::startTracing()
{
    if (m_running.load()) return;

    m_running.store(true);

    std::thread worker([this]() {
        traceRays();
        m_running.store(false);
    });

    worker.detach();
}

void RayTracerEngine::cancelTracing()
{
    m_cancelFlag.store(true);
    while (m_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool RayTracerEngine::isRunning() const
{
    return m_running.load();
}

std::vector<RayPath> RayTracerEngine::rayPaths() const
{
    return m_rayPaths;
}

int RayTracerEngine::progress() const
{
    return m_progress.load();
}
