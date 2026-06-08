#include "NURBSSurface.h"
#include <cmath>
#include <algorithm>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

NURBSSurface::NURBSSurface() = default;

void NURBSSurface::setDegree(int degreeU, int degreeV)
{
    m_degreeU = degreeU;
    m_degreeV = degreeV;
}

void NURBSSurface::setControlPoints(const std::vector<std::vector<QVector3D>>& points)
{
    m_controlPoints = points;
    if (m_weights.size() != points.size() ||
        (m_weights.empty() && !points.empty()) ||
        (m_weights.size() == points.size() && !points.empty() && m_weights[0].size() != points[0].size())) {
        m_weights.resize(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            m_weights[i].resize(points[i].size(), 1.0f);
        }
    }
}

void NURBSSurface::setWeights(const std::vector<std::vector<float>>& weights)
{
    m_weights = weights;
}

void NURBSSurface::setKnotVectors(const std::vector<float>& knotsU, const std::vector<float>& knotsV)
{
    m_knotsU = knotsU;
    m_knotsV = knotsV;
}

int NURBSSurface::findSpan(int degree, const std::vector<float>& knots, int n, float t) const
{
    if (t >= knots[n + 1]) return n;
    if (t <= knots[degree]) return degree;

    int lo = degree;
    int hi = n + 1;
    int mid = (lo + hi) / 2;

    while (t < knots[mid] || t >= knots[mid + 1]) {
        if (t < knots[mid])
            hi = mid;
        else
            lo = mid;
        mid = (lo + hi) / 2;
    }
    return mid;
}

float NURBSSurface::basisFunction(int degree, const std::vector<float>& knots, int i, float t) const
{
    if (degree == 0) {
        return (t >= knots[i] && t < knots[i + 1]) ? 1.0f : 0.0f;
    }

    float denom1 = knots[i + degree] - knots[i];
    float denom2 = knots[i + degree + 1] - knots[i + 1];

    float c1 = 0.0f, c2 = 0.0f;

    if (std::abs(denom1) > 1e-10f)
        c1 = ((t - knots[i]) / denom1) * basisFunction(degree - 1, knots, i, t);

    if (std::abs(denom2) > 1e-10f)
        c2 = ((knots[i + degree + 1] - t) / denom2) * basisFunction(degree - 1, knots, i + 1, t);

    return c1 + c2;
}

float NURBSSurface::basisFunctionDerivative(int degree, const std::vector<float>& knots, int i, int derivOrder, float t) const
{
    if (derivOrder == 0)
        return basisFunction(degree, knots, i, t);

    float denom1 = knots[i + degree] - knots[i];
    float denom2 = knots[i + degree + 1] - knots[i + 1];

    float c1 = 0.0f, c2 = 0.0f;

    if (std::abs(denom1) > 1e-10f)
        c1 = (float(degree) / denom1) * basisFunctionDerivative(degree - 1, knots, i, derivOrder - 1, t);

    if (std::abs(denom2) > 1e-10f)
        c2 = (float(degree) / denom2) * basisFunctionDerivative(degree - 1, knots, i + 1, derivOrder - 1, t);

    return c1 - c2;
}

QVector3D NURBSSurface::evaluate(float u, float v) const
{
    int nU = controlPointCountU() - 1;
    int nV = controlPointCountV() - 1;

    if (nU < m_degreeU || nV < m_degreeV)
        return QVector3D(0, 0, 0);

    int spanU = findSpan(m_degreeU, m_knotsU, nU, u);
    int spanV = findSpan(m_degreeV, m_knotsV, nV, v);

    QVector3D numerator(0, 0, 0);
    float denominator = 0.0f;

    for (int i = 0; i <= m_degreeU; ++i) {
        int idxU = spanU - m_degreeU + i;
        if (idxU < 0 || idxU >= controlPointCountU()) continue;

        float Nu = basisFunction(m_degreeU, m_knotsU, idxU, u);

        for (int j = 0; j <= m_degreeV; ++j) {
            int idxV = spanV - m_degreeV + j;
            if (idxV < 0 || idxV >= controlPointCountV()) continue;

            float Nv = basisFunction(m_degreeV, m_knotsV, idxV, v);
            float w = m_weights[idxU][idxV];
            float coeff = Nu * Nv * w;

            numerator += coeff * m_controlPoints[idxU][idxV];
            denominator += coeff;
        }
    }

    if (std::abs(denominator) < 1e-10f)
        return QVector3D(0, 0, 0);

    return numerator / denominator;
}

void NURBSSurface::evaluateDerivatives(float u, float v, QVector3D& dPdU, QVector3D& dPdV) const
{
    int nU = controlPointCountU() - 1;
    int nV = controlPointCountV() - 1;

    if (nU < m_degreeU || nV < m_degreeV) {
        dPdU = dPdV = QVector3D(0, 0, 0);
        return;
    }

    int spanU = findSpan(m_degreeU, m_knotsU, nU, u);
    int spanV = findSpan(m_degreeV, m_knotsV, nV, v);

    QVector3D S(0, 0, 0), S_u(0, 0, 0), S_v(0, 0, 0);
    float w = 0.0f, w_u = 0.0f, w_v = 0.0f;

    for (int i = 0; i <= m_degreeU; ++i) {
        int idxU = spanU - m_degreeU + i;
        if (idxU < 0 || idxU >= controlPointCountU()) continue;

        float Nu = basisFunction(m_degreeU, m_knotsU, idxU, u);
        float dNu = basisFunctionDerivative(m_degreeU, m_knotsU, idxU, 1, u);

        for (int j = 0; j <= m_degreeV; ++j) {
            int idxV = spanV - m_degreeV + j;
            if (idxV < 0 || idxV >= controlPointCountV()) continue;

            float Nv = basisFunction(m_degreeV, m_knotsV, idxV, v);
            float dNv = basisFunctionDerivative(m_degreeV, m_knotsV, idxV, 1, v);

            float wt = m_weights[idxU][idxV];
            float cuv = Nu * Nv * wt;
            float duv = dNu * Nv * wt;
            float dvv = Nu * dNv * wt;

            const QVector3D& P = m_controlPoints[idxU][idxV];
            S += cuv * P;
            S_u += duv * P;
            S_v += dvv * P;
            w += cuv;
            w_u += duv;
            w_v += dvv;
        }
    }

    if (std::abs(w) < 1e-10f) {
        dPdU = dPdV = QVector3D(0, 0, 0);
        return;
    }

    float invW = 1.0f / w;
    dPdU = (S_u - S * (w_u * invW)) * invW;
    dPdV = (S_v - S * (w_v * invW)) * invW;
}

QVector3D NURBSSurface::normal(float u, float v) const
{
    QVector3D dPdU, dPdV;
    evaluateDerivatives(u, v, dPdU, dPdV);
    QVector3D n = QVector3D::crossProduct(dPdU, dPdV);
    float len = n.length();
    if (len < 1e-10f)
        return QVector3D(0, 1, 0);
    return n / len;
}

NURBSSurface NURBSSurface::createAsphericLens(float radius, float conicConstant, float apertureRadius, int resolution)
{
    NURBSSurface surf;
    surf.setDegree(3, 3);

    int nU = resolution + 3;
    int nV = resolution + 3;

    std::vector<std::vector<QVector3D>> ctrlPts(nU, std::vector<QVector3D>(nV));
    std::vector<std::vector<float>> wts(nU, std::vector<float>(nV, 1.0f));

    for (int i = 0; i < nU; ++i) {
        float u = float(i) / float(nU - 1);
        float theta = u * float(M_PI) * 0.5f;
        float r = apertureRadius * std::sin(theta);

        for (int j = 0; j < nV; ++j) {
            float v = float(j) / float(nV - 1);
            float phi = v * 2.0f * float(M_PI);

            float px = r * std::cos(phi);
            float py = r * std::sin(phi);
            float r2 = r * r;
            float z = (r2 / radius) / (1.0f + std::sqrt(1.0f - (1.0f + conicConstant) * r2 / (radius * radius)));

            ctrlPts[i][j] = QVector3D(px, py, -z);
        }
    }

    surf.setControlPoints(ctrlPts);
    surf.setWeights(wts);

    int degU = 3, degV = 3;

    std::vector<float> knotsU(nU + degU + 1, 0.0f);
    for (int i = 0; i <= nU + degU; ++i) {
        if (i <= degU)
            knotsU[i] = 0.0f;
        else if (i >= nU)
            knotsU[i] = 1.0f;
        else
            knotsU[i] = float(i - degU) / float(nU - degU);
    }

    std::vector<float> knotsV(nV + degV + 1, 0.0f);
    for (int i = 0; i <= nV + degV; ++i) {
        if (i <= degV)
            knotsV[i] = 0.0f;
        else if (i >= nV)
            knotsV[i] = 1.0f;
        else
            knotsV[i] = float(i - degV) / float(nV - degV);
    }

    surf.setKnotVectors(knotsU, knotsV);
    return surf;
}

NURBSSurface NURBSSurface::createFreeformLens(float baseRadius, float freeformAmplitude, float apertureRadius, int resolution)
{
    NURBSSurface surf;
    surf.setDegree(3, 3);

    int nU = resolution + 3;
    int nV = resolution + 3;

    std::vector<std::vector<QVector3D>> ctrlPts(nU, std::vector<QVector3D>(nV));
    std::vector<std::vector<float>> wts(nU, std::vector<float>(nV, 1.0f));

    for (int i = 0; i < nU; ++i) {
        float u = float(i) / float(nU - 1);
        float theta = u * float(M_PI) * 0.5f;
        float r = apertureRadius * std::sin(theta);

        for (int j = 0; j < nV; ++j) {
            float v = float(j) / float(nV - 1);
            float phi = v * 2.0f * float(M_PI);

            float px = r * std::cos(phi);
            float py = r * std::sin(phi);
            float r2 = r * r;
            float z = (r2 / baseRadius) / (1.0f + std::sqrt(std::max(1e-6f, 1.0f - r2 / (baseRadius * baseRadius))));

            float freeform = freeformAmplitude * std::sin(3.0f * phi) * (r / apertureRadius) * (r / apertureRadius);
            z += freeform;

            ctrlPts[i][j] = QVector3D(px, py, -z);
            wts[i][j] = 1.0f + 0.2f * std::sin(2.0f * phi) * u;
        }
    }

    surf.setControlPoints(ctrlPts);
    surf.setWeights(wts);

    int degU2 = 3, degV2 = 3;

    std::vector<float> knotsU2(nU + degU2 + 1, 0.0f);
    for (int i = 0; i <= nU + degU2; ++i) {
        if (i <= degU2)
            knotsU2[i] = 0.0f;
        else if (i >= nU)
            knotsU2[i] = 1.0f;
        else
            knotsU2[i] = float(i - degU2) / float(nU - degU2);
    }

    std::vector<float> knotsV2(nV + degV2 + 1, 0.0f);
    for (int i = 0; i <= nV + degV2; ++i) {
        if (i <= degV2)
            knotsV2[i] = 0.0f;
        else if (i >= nV)
            knotsV2[i] = 1.0f;
        else
            knotsV2[i] = float(i - degV2) / float(nV - degV2);
    }

    surf.setKnotVectors(knotsU2, knotsV2);
    return surf;
}
