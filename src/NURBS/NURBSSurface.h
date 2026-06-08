#pragma once

#include <QVector3D>
#include <QVector2D>
#include <vector>
#include <string>

class NURBSSurface
{
public:
    NURBSSurface();

    void setDegree(int degreeU, int degreeV);
    void setControlPoints(const std::vector<std::vector<QVector3D>>& points);
    void setWeights(const std::vector<std::vector<float>>& weights);
    void setKnotVectors(const std::vector<float>& knotsU, const std::vector<float>& knotsV);

    int degreeU() const { return m_degreeU; }
    int degreeV() const { return m_degreeV; }
    int controlPointCountU() const { return static_cast<int>(m_controlPoints.size()); }
    int controlPointCountV() const { return m_controlPoints.empty() ? 0 : static_cast<int>(m_controlPoints[0].size()); }
    const std::vector<std::vector<QVector3D>>& controlPoints() const { return m_controlPoints; }
    const std::vector<std::vector<float>>& weights() const { return m_weights; }
    const std::vector<float>& knotVectorU() const { return m_knotsU; }
    const std::vector<float>& knotVectorV() const { return m_knotsV; }

    QVector3D evaluate(float u, float v) const;
    QVector3D normal(float u, float v) const;
    void evaluateDerivatives(float u, float v, QVector3D& dPdU, QVector3D& dPdV) const;

    static NURBSSurface createAsphericLens(float radius, float conicConstant, float apertureRadius, int resolution = 8);
    static NURBSSurface createFreeformLens(float baseRadius, float freeformAmplitude, float apertureRadius, int resolution = 8);

private:
    float basisFunction(int degree, const std::vector<float>& knots, int i, float t) const;
    float basisFunctionDerivative(int degree, const std::vector<float>& knots, int i, int derivOrder, float t) const;
    int findSpan(int degree, const std::vector<float>& knots, int n, float t) const;

    int m_degreeU = 3;
    int m_degreeV = 3;
    std::vector<std::vector<QVector3D>> m_controlPoints;
    std::vector<std::vector<float>> m_weights;
    std::vector<float> m_knotsU;
    std::vector<float> m_knotsV;
};
