#include <vector>
#include <cmath>
#include <glad/glad.h>
#include "Mesh.h"

// ------------------------------------------------------------
// Cylinder mesh for branches
// position (loc 0), normal (loc 1)
// ------------------------------------------------------------
Mesh createCylinderMesh(int segments)
{
    std::vector<float> verts;
    verts.reserve(segments * 6 * 6);

    const float radius = 0.5f;
    const float height = 1.0f;
    const float twoPi  = 6.28318530718f;

    for (int i = 0; i < segments; i++) {
        float a0 = (float)i / segments * twoPi;
        float a1 = (float)(i + 1) / segments * twoPi;

        float x0 = std::cos(a0), z0 = std::sin(a0);
        float x1 = std::cos(a1), z1 = std::sin(a1);

        float quad[] = {
            // pos                      // normal
            radius*x0, 0.0f,    radius*z0,   x0, 0.0f, z0,
            radius*x1, 0.0f,    radius*z1,   x1, 0.0f, z1,
            radius*x0, height,  radius*z0,   x0, 0.0f, z0,

            radius*x1, 0.0f,    radius*z1,   x1, 0.0f, z1,
            radius*x1, height,  radius*z1,   x1, 0.0f, z1,
            radius*x0, height,  radius*z0,   x0, 0.0f, z0
        };

        verts.insert(verts.end(), quad, quad + 36);
    }

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    // position (0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal (1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    m.vertexCount = verts.size() / 6;
    m.mode = GL_TRIANGLES;
    return m;
}

// ------------------------------------------------------------
// Leaf cluster mesh
// position (loc 0), UV (loc 1)
// ------------------------------------------------------------
Mesh createLeafClusterMesh()
{
    const float u0 = 0.0f;
    const float u1 = 0.25f;
    const float v0 = 0.0f;
    const float v1 = 0.25f;

    float verts[] = {
        // Quad 1
        -0.6f, 0.0f, 0.0f,   u0, v0,
         0.6f, 0.0f, 0.0f,   u1, v0,
        -0.3f, 1.0f, 0.0f,   u0, v1,
         0.3f, 1.0f, 0.0f,   u1, v1,

        // Quad 2
        -0.5f, 0.0f, 0.0f,   u0, v0,
         0.5f, 0.0f, 0.0f,   u1, v0,
        -0.25f,1.0f, 0.0f,   u0, v1,
         0.25f, 1.0f, 0.0f,  u1, v1,

        // Quad 3
        -0.3f, 0.0f, 0.0f,   u0, v0,
         0.3f, 0.0f, 0.0f,   u1, v0,
        -0.15f,1.0f, 0.0f,   u0, v1,
         0.15f, 1.0f, 0.0f,  u1, v1
    };

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // position (0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // UV (1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    m.vertexCount = 12;
    m.mode = GL_TRIANGLE_STRIP;
    return m;
}

// ------------------------------------------------------------
// Ground plane mesh
// ------------------------------------------------------------
Mesh createGroundPlaneMesh()
{
    float verts[] = {
        -50.0f, 0.0f, -50.0f,
         50.0f, 0.0f, -50.0f,
        -50.0f, 0.0f,  50.0f,
         50.0f, 0.0f,  50.0f
    };

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    m.vertexCount = 4;
    m.mode = GL_TRIANGLE_STRIP;
    return m;
}
