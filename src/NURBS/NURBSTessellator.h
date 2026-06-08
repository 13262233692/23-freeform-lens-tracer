#pragma once

#include <QVector3D>
#include <vector>

struct MeshData
{
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<unsigned int> indices;
    void clear()
    {
        vertices.clear();
        normals.clear();
        indices.clear();
    }
    bool isEmpty() const { return vertices.empty(); }
    int vertexCount() const { return static_cast<int>(vertices.size()) / 3; }
    int triangleCount() const { return static_cast<int>(indices.size()) / 3; }
};

class NURBSTessellator
{
public:
    NURBSTessellator();

    void setResolution(int resU, int resV);
    int resolutionU() const { return m_resU; }
    int resolutionV() const { return m_resV; }

    MeshData tessellate(const class NURBSSurface& surface) const;

private:
    int m_resU = 64;
    int m_resV = 64;
};
