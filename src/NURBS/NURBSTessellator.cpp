#include "NURBSTessellator.h"
#include "NURBSSurface.h"

NURBSTessellator::NURBSTessellator() = default;

void NURBSTessellator::setResolution(int resU, int resV)
{
    m_resU = std::max(4, resU);
    m_resV = std::max(4, resV);
}

MeshData NURBSTessellator::tessellate(const NURBSSurface& surface) const
{
    MeshData mesh;

    int vertCountU = m_resU + 1;
    int vertCountV = m_resV + 1;

    mesh.vertices.reserve(vertCountU * vertCountV * 3);
    mesh.normals.reserve(vertCountU * vertCountV * 3);

    for (int i = 0; i <= m_resU; ++i) {
        float u = float(i) / float(m_resU);
        for (int j = 0; j <= m_resV; ++j) {
            float v = float(j) / float(m_resV);

            QVector3D pos = surface.evaluate(u, v);
            QVector3D nrm = surface.normal(u, v);

            mesh.vertices.push_back(pos.x());
            mesh.vertices.push_back(pos.y());
            mesh.vertices.push_back(pos.z());
            mesh.normals.push_back(nrm.x());
            mesh.normals.push_back(nrm.y());
            mesh.normals.push_back(nrm.z());
        }
    }

    mesh.indices.reserve(m_resU * m_resV * 6);

    for (int i = 0; i < m_resU; ++i) {
        for (int j = 0; j < m_resV; ++j) {
            unsigned int v00 = i * vertCountV + j;
            unsigned int v10 = (i + 1) * vertCountV + j;
            unsigned int v01 = i * vertCountV + (j + 1);
            unsigned int v11 = (i + 1) * vertCountV + (j + 1);

            mesh.indices.push_back(v00);
            mesh.indices.push_back(v10);
            mesh.indices.push_back(v01);

            mesh.indices.push_back(v10);
            mesh.indices.push_back(v11);
            mesh.indices.push_back(v01);
        }
    }

    return mesh;
}
