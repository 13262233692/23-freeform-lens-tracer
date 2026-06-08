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

float cauchyRefractiveIndex(float n_d, float abbeV, float wavelength_nm)
{
    float lambda_d = WAVELENGTH_d * 0.001f;
    float lambda_F = WAVELENGTH_F * 0.001f;
    float lambda_C = WAVELENGTH_C * 0.001f;

    float invLamF2 = 1.0f / (lambda_F * lambda_F);
    float invLamC2 = 1.0f / (lambda_C * lambda_C);

    float deltaN = (n_d - 1.0f) / abbeV;

    float B = deltaN / (invLamF2 - invLamC2);
    float A = n_d - B / (lambda_d * lambda_d);

    float lambda = wavelength_nm * 0.001f;
    float invLam2 = 1.0f / (lambda * lambda);

    return A + B * invLam2;
}

static float spectralIntensityCorrection(float wavelength_nm)
{
    float x = (wavelength_nm - 380.0f) / 40.0f;
    if (wavelength_nm < 420.0f) return 0.3f + 0.7f * x;
    if (wavelength_nm > 700.0f) return 0.3f + 0.7f * (780.0f - wavelength_nm) / 80.0f;
    return 1.0f;
}

QVector3D wavelengthToRGB(float wavelength_nm)
{
    float r = 0.0f, g = 0.0f, b = 0.0f;

    if (wavelength_nm >= 380.0f && wavelength_nm < 440.0f) {
        r = -(wavelength_nm - 440.0f) / (440.0f - 380.0f);
        b = 1.0f;
    } else if (wavelength_nm >= 440.0f && wavelength_nm < 490.0f) {
        g = (wavelength_nm - 440.0f) / (490.0f - 440.0f);
        b = 1.0f;
    } else if (wavelength_nm >= 490.0f && wavelength_nm < 510.0f) {
        g = 1.0f;
        b = -(wavelength_nm - 510.0f) / (510.0f - 490.0f);
    } else if (wavelength_nm >= 510.0f && wavelength_nm < 580.0f) {
        r = (wavelength_nm - 510.0f) / (580.0f - 510.0f);
        g = 1.0f;
    } else if (wavelength_nm >= 580.0f && wavelength_nm < 645.0f) {
        r = 1.0f;
        g = -(wavelength_nm - 645.0f) / (645.0f - 580.0f);
    } else if (wavelength_nm >= 645.0f && wavelength_nm <= 780.0f) {
        r = 1.0f;
    }

    float factor = spectralIntensityCorrection(wavelength_nm);

    r = std::pow(r * factor, 0.8f);
    g = std::pow(g * factor, 0.8f);
    b = std::pow(b * factor, 0.8f);

    return QVector3D(
        std::max(0.05f, std::min(1.0f, r)),
        std::max(0.05f, std::min(1.0f, g)),
        std::max(0.05f, std::min(1.0f, b)));
}

float sampleSpectralWavelength()
{
    return WAVELENGTH_MIN + randomFloat() * (WAVELENGTH_MAX - WAVELENGTH_MIN);
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
