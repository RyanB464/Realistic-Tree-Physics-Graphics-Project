#pragma once
#include <glad/glad.h>

struct Mesh {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int vertexCount = 0;
    GLenum mode = GL_TRIANGLES;
};

Mesh createCylinderMesh(int segments);
Mesh createLeafClusterMesh();
Mesh createGroundPlaneMesh();
