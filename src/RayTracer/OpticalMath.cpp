#include "OpticalMath.h"
#include <random>
#include <algorithm>

namespace OpticalMath
{

static thread_local std::mt19937 s_generator{std::random_device{}()};
static thread_local std::uniform_real_distribution<float> s_distribution(0.0f, 1.0f);

QVector3D refract(const QVector3D& incident, const QVector3D& normal, float n1, float n2)
{
    float cosI = -QVector3D::dotProduct(incident, normal);
    float ratio = n1 / n2;
    float sinT2 = ratio * ratio * (1.0f - cosI * cosI);

    if (sinT2 > 1.0f)
        return QVector3D(0, 0, 0);

    float cosT = std::sqrt(1.0f - sinT2);
    return (ratio * incident + (ratio * cosI - cosT) * normal).normalized();
}

QVector3D reflect(const QVector3D& incident, const QVector3D& normal)
{
    float cosI = -QVector3D::dotProduct(incident, normal);
    return (incident + 2.0f * cosI * normal).normalized();
}

float fresnelReflectance(const QVector3D& incident, const QVector3D& normal, float n1, float n2)
{
    float cosI = -QVector3D::dotProduct(incident, normal);
    float sinT2 = (n1 / n2) * (n1 / n2) * (1.0f - cosI * cosI);

    if (sinT2 > 1.0f) return 1.0f;

    float cosT = std::sqrt(1.0f - sinT2);
    float rs = ((n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT));
    float rp = ((n1 * cosT - n2 * cosI) / (n1 * cosT + n2 * cosI));

    return (rs * rs + rp * rp) * 0.5f;
}

bool totalInternalReflection(const QVector3D& incident, const QVector3D& normal, float n1, float n2)
{
    float cosI = -QVector3D::dotProduct(incident, normal);
    float sinT2 = (n1 / n2) * (n1 / n2) * (1.0f - cosI * cosI);
    return sinT2 > 1.0f;
}

float randomFloat()
{
    return s_distribution(s_generator);
}

QVector3D randomDirectionCone(float halfAngle)
{
    float cosHalf = std::cos(halfAngle);
    float r1 = randomFloat();
    float r2 = randomFloat();

    float z = 1.0f + r1 * (cosHalf - 1.0f);
    float phi = 2.0f * 3.14159265358979323846f * r2;
    float sinTheta = std::sqrt(1.0f - z * z);

    return QVector3D(sinTheta * std::cos(phi), sinTheta * std::sin(phi), z);
}

QVector3D randomDirectionHemisphere(const QVector3D& normal)
{
    float r1 = randomFloat();
    float r2 = randomFloat();

    float phi = 2.0f * 3.14159265358979323846f * r1;
    float cosTheta = std::sqrt(r2);
    float sinTheta = std::sqrt(1.0f - r2);

    QVector3D localDir(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

    QVector3D up = std::abs(normal.z()) < 0.999f ? QVector3D(0, 0, 1) : QVector3D(1, 0, 0);
    QVector3D tangent = QVector3D::crossProduct(normal, up).normalized();
    QVector3D bitangent = QVector3D::crossProduct(normal, tangent);

    return (tangent * localDir.x() + bitangent * localDir.y() + normal * localDir.z()).normalized();
}

}
