#pragma once

#include <QVector3D>
#include <cmath>

namespace OpticalMath
{

constexpr float WAVELENGTH_F = 486.13f;
constexpr float WAVELENGTH_d = 587.56f;
constexpr float WAVELENGTH_C = 656.27f;
constexpr float WAVELENGTH_MIN = 380.0f;
constexpr float WAVELENGTH_MAX = 780.0f;

struct Ray
{
    QVector3D origin;
    QVector3D direction;
    float wavelength = 589.0f;

    Ray() = default;
    Ray(const QVector3D& o, const QVector3D& d, float wl = 589.0f)
        : origin(o), direction(d.normalized()), wavelength(wl) {}

    QVector3D pointAt(float t) const { return origin + direction * t; }
};

struct HitRecord
{
    float t = -1.0f;
    QVector3D point;
    QVector3D normal;
    float u = 0.0f;
    float v = 0.0f;
    bool hit = false;
};

QVector3D refract(const QVector3D& incident, const QVector3D& normal, float n1, float n2);
QVector3D reflect(const QVector3D& incident, const QVector3D& normal);
float fresnelReflectance(const QVector3D& incident, const QVector3D& normal, float n1, float n2);
bool totalInternalReflection(const QVector3D& incident, const QVector3D& normal, float n1, float n2);

float cauchyRefractiveIndex(float n_d, float abbeV, float wavelength_nm);
QVector3D wavelengthToRGB(float wavelength_nm);
float sampleSpectralWavelength();

float randomFloat();
QVector3D randomDirectionCone(float halfAngle);
QVector3D randomDirectionHemisphere(const QVector3D& normal);

}
