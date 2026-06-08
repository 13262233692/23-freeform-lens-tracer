#pragma once

#include <QVector3D>
#include <QMatrix4x4>
#include <QPoint>

class Camera
{
public:
    Camera();

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix(float aspectRatio) const;

    void orbit(const QPointF& delta);
    void zoom(float delta);
    void pan(const QPointF& delta);

    QVector3D position() const { return m_position; }
    QVector3D target() const { return m_target; }
    QVector3D up() const { return m_up; }

    void setTarget(const QVector3D& target);
    void setPosition(const QVector3D& pos);

    float fieldOfView() const { return m_fov; }
    void setFieldOfView(float fov);

    float nearPlane() const { return m_nearPlane; }
    float farPlane() const { return m_farPlane; }

private:
    void updateViewMatrix();

    QVector3D m_position{5.0f, 3.0f, 8.0f};
    QVector3D m_target{0.0f, 0.0f, 0.0f};
    QVector3D m_up{0.0f, 1.0f, 0.0f};

    float m_fov = 45.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 100.0f;

    float m_orbitSpeed = 0.005f;
    float m_panSpeed = 0.01f;
    float m_zoomSpeed = 0.1f;
};
