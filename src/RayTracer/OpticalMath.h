#pragma once

#include <QVector3D>
#include <cmath>

namespace OpticalMath
{

struct Ray
{
    QVector3D origin;
    QVector3D direction;

    Ray() = default;
    Ray(const QVector3D& o, const QVector3D& d) : origin(o), direction(d.normalized()) {}

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

float randomFloat();
QVector3D randomDirectionCone(float halfAngle);
QVector3D randomDirectionHemisphere(const QVector3D& normal);

}
