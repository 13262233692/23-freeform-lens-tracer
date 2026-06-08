#include "Camera.h"
#include <cmath>

Camera::Camera() = default;

QMatrix4x4 Camera::viewMatrix() const
{
    QMatrix4x4 mat;
    mat.lookAt(m_position, m_target, m_up);
    return mat;
}

QMatrix4x4 Camera::projectionMatrix(float aspectRatio) const
{
    QMatrix4x4 mat;
    mat.perspective(m_fov, aspectRatio, m_nearPlane, m_farPlane);
    return mat;
}

void Camera::orbit(const QPointF& delta)
{
    QVector3D viewDir = (m_target - m_position).normalized();
    QVector3D right = QVector3D::crossProduct(viewDir, m_up).normalized();

    float azimuthDelta = float(delta.x()) * m_orbitSpeed;
    float elevationDelta = float(delta.y()) * m_orbitSpeed;

    QMatrix4x4 rotAzimuth;
    rotAzimuth.rotate(azimuthDelta * 57.2957795f, m_up);

    QMatrix4x4 rotElevation;
    rotElevation.rotate(elevationDelta * 57.2957795f, right);

    m_position = rotElevation * rotAzimuth * m_position;
    m_up = rotElevation * rotAzimuth * m_up;
    m_up.normalize();
}

void Camera::zoom(float delta)
{
    QVector3D viewDir = (m_target - m_position).normalized();
    float dist = (m_target - m_position).length();
    float newDist = dist * (1.0f - delta * m_zoomSpeed);
    newDist = std::max(0.5f, std::min(50.0f, newDist));
    m_position = m_target - viewDir * newDist;
}

void Camera::pan(const QPointF& delta)
{
    QVector3D viewDir = (m_target - m_position).normalized();
    QVector3D right = QVector3D::crossProduct(viewDir, m_up).normalized();
    QVector3D trueUp = QVector3D::crossProduct(right, viewDir).normalized();

    float dist = (m_target - m_position).length();
    QVector3D offset = (-right * float(delta.x()) + trueUp * float(delta.y())) * dist * m_panSpeed;

    m_position += offset;
    m_target += offset;
}

void Camera::setTarget(const QVector3D& target)
{
    m_target = target;
}

void Camera::setPosition(const QVector3D& pos)
{
    m_position = pos;
}

void Camera::setFieldOfView(float fov)
{
    m_fov = std::max(5.0f, std::min(120.0f, fov));
}
