#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>
#include <cmath>

Mesh createCylinderMesh(int segments) {
    std::vector<float> verts;
    float radius = 1.0f;
    float height = 1.0f;

    for (int i = 0; i < segments; i++) {
        float t1 = (float)i / segments * 2.0f * 3.14159f;
        float t2 = (float)(i + 1) / segments * 2.0f * 3.14159f;

        float x1 = radius * cos(t1);
        float z1 = radius * sin(t1);
        float x2 = radius * cos(t2);
        float z2 = radius * sin(t2);

        glm::vec3 n1 = glm::normalize(glm::vec3(x1, 0.0f, z1));
        glm::vec3 n2 = glm::normalize(glm::vec3(x2, 0.0f, z2));

        // tri 1
        verts.insert(verts.end(), { x1, 0.0f, z1, n1.x, n1.y, n1.z });
        verts.insert(verts.end(), { x2, 0.0f, z2, n2.x, n2.y, n2.z });
        verts.insert(verts.end(), { x1, height, z1, n1.x, n1.y, n1.z });

        // tri 2
        verts.insert(verts.end(), { x2, 0.0f, z2, n2.x, n2.y, n2.z });
        verts.insert(verts.end(), { x2, height, z2, n2.x, n2.y, n2.z });
        verts.insert(verts.end(), { x1, height, z1, n1.x, n1.y, n1.z });
    }

    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    m.vertexCount = static_cast<unsigned int>(verts.size() / 6);
    m.mode = GL_TRIANGLES;
    return m;
}

Mesh createLeafClusterMesh() {
    float leafCluster[] = {
    // Quad 1 (wide oval)
    -0.6f, 0.0f, 0.0f,
     0.6f, 0.0f, 0.0f,
    -0.3f, 1.0f, 0.0f,
     0.3f, 1.0f, 0.0f,

    // Quad 2 (rotated)
    -0.5f, 0.0f, 0.0f,
     0.5f, 0.0f, 0.0f,
    -0.25f, 1.0f, 0.0f,
     0.25f, 1.0f, 0.0f,

    // Quad 3 (narrow)
    -0.3f, 0.0f, 0.0f,
     0.3f, 0.0f, 0.0f,
    -0.15f, 1.0f, 0.0f,
     0.15f, 1.0f, 0.0f
    };


    Mesh m;
    glGenVertexArrays(1, &m.VAO);
    glGenBuffers(1, &m.VBO);

    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leafCluster), leafCluster, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    m.vertexCount = 12; // 3 quads * 4 verts
    m.mode = GL_TRIANGLE_STRIP; // we’ll draw each quad separately
    return m;
}

Mesh createGroundPlaneMesh() {
    float groundVerts[] = {
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVerts), groundVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    m.vertexCount = 4;
    m.mode = GL_TRIANGLE_STRIP;
    return m;
}
