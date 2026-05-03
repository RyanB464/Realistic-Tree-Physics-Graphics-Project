// Tree.cpp
#include "Tree.h"
#include "LSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <iostream>
#include <iomanip>
#include <GLFW/glfw3.h>

// ------------------------------------------------------------
// Simple bounding-sphere frustum test
// ------------------------------------------------------------
static bool sphereInFrustum(const glm::mat4& VP, const glm::vec3& center, float radius)
{
    glm::vec4 c = VP * glm::vec4(center, 1.0f);
    float w = fabs(c.w);

    if (c.x < -w - radius) return false;
    if (c.x >  w + radius) return false;
    if (c.y < -w - radius) return false;
    if (c.y >  w + radius) return false;
    if (c.z < -w - radius) return false;
    if (c.z >  w + radius) return false;

    return true;
}

// ------------------------------------------------------------
// Tree generation
// ------------------------------------------------------------
void Tree::generate(const std::string& axiom, int iterations,
                    float baseLength, float baseRadius)
{
    branches.clear();
    leaves.clear();

    std::string seq = LSystem::generate(axiom, iterations);
    LSystem::buildTree(seq, branches, leaves, baseLength, baseRadius);

    worldMatrix = glm::mat4(1.0f);

    // Connectivity debug
    const float warnThreshold = 0.05f;
    bool anyWarn = false;

    for (size_t i = 0; i < branches.size(); ++i) {
        const Branch& b = branches[i];
        if (b.parentIndex >= 0 && b.parentIndex < (int)branches.size()) {
            const Branch& p = branches[b.parentIndex];

            glm::vec3 baseWorld = glm::vec3(b.worldTransform[3]);
            glm::vec3 parentTip = glm::vec3(p.worldTransform * glm::vec4(p.localTranslation, 1.0f));
            float dist = glm::length(baseWorld - parentTip);

            if (dist > warnThreshold) {
                if (!anyWarn) {
                    std::cout << "Connectivity warnings:\n";
                    anyWarn = true;
                }
                std::cout << "Branch[" << i << "] parent=" << b.parentIndex
                          << " base=(" << baseWorld.x << "," << baseWorld.y << "," << baseWorld.z << ")"
                          << " parentTip=(" << parentTip.x << "," << parentTip.y << "," << parentTip.z << ")"
                          << " dist=" << dist << "\n";
            }
        }
    }

    if (anyWarn) std::cout << std::flush;
}

// ------------------------------------------------------------
// Tree draw (AABB culling + branch instancing + leaf instancing)
// ------------------------------------------------------------
void Tree::draw(const Shader& branchShader,
                const Shader& leafShader,
                const glm::mat4& view,
                const glm::mat4& projection,
                const Physics& physics,
                const Mesh& branchMesh,
                const Mesh& leafMesh) const
{
    glm::mat4 VP = projection * view;

    // --------------------------------------------------------
    // Compute AABB from branch world positions
    // --------------------------------------------------------
    glm::vec3 minB( 1e9f);
    glm::vec3 maxB(-1e9f);

    for (const Branch& b : branches) {
        glm::vec3 p = glm::vec3(worldMatrix * b.worldTransform[3]);
        minB = glm::min(minB, p);
        maxB = glm::max(maxB, p);
    }

    glm::vec3 treeCenter = 0.5f * (minB + maxB);
    float treeRadius = glm::length(maxB - treeCenter);

    // Safety padding
    treeRadius *= 1.20f;

    if (!sphereInFrustum(VP, treeCenter, treeRadius))
        return;

    // --------------------------------------------------------
    // Branch rendering (instanced)
    // --------------------------------------------------------
    branchShader.use();
    branchShader.setMat4("view", view);
    branchShader.setMat4("projection", projection);
    branchShader.setVec3("lightDir", glm::normalize(glm::vec3(-0.3f, -1.0f, -0.2f)));
    branchShader.setVec3("lightColor", glm::vec3(1.0f));
    branchShader.setVec3("baseColor", glm::vec3(0.55f, 0.35f, 0.2f));

    glBindVertexArray(branchMesh.VAO);

    const float snapThreshold = 0.12f;
    const float minSnapEpsilon = 1e-6f;
    const float blendSpeed = 8.0f;
    const float capRadius = 0.035f;

    float time = (float)glfwGetTime();
    static float lastTime = 0.0f;
    float dt = (lastTime > 0.0f) ? (time - lastTime) : (1.0f / 60.0f);
    lastTime = time;
    float blendFactor = 1.0f - expf(-blendSpeed * dt);

    // Build instance matrices
    std::vector<glm::mat4> branchModels;
    branchModels.reserve(branches.size());

    for (const Branch& b : branches) {
        glm::vec3 baseWorldPos = glm::vec3(b.worldTransform[3]);

        if (b.parentIndex >= 0 && b.parentIndex < (int)branches.size()) {
            const Branch& p = branches[b.parentIndex];
            glm::vec3 parentTipWorld =
                glm::vec3(p.worldTransform * glm::vec4(p.localTranslation, 1.0f));

            float gap = glm::length(baseWorldPos - parentTipWorld);

            if (gap > minSnapEpsilon && gap <= snapThreshold)
                baseWorldPos = glm::mix(baseWorldPos, parentTipWorld, blendFactor);
            else if (gap <= minSnapEpsilon)
                baseWorldPos = parentTipWorld;
        }

        glm::mat3 rot3 = glm::mat3(b.worldTransform);
        glm::mat4 rotationOnly = glm::mat4(rot3);
        rotationOnly[3] = glm::vec4(0,0,0,1);

        glm::mat4 worldPlacement =
            worldMatrix * glm::translate(glm::mat4(1.0f), baseWorldPos);

        glm::mat4 localGeom = rotationOnly;
        localGeom = glm::scale(localGeom, glm::vec3(b.radius, b.length, b.radius));

        branchModels.push_back(worldPlacement * localGeom);
    }

    // Upload instance matrices
    static GLuint branchInstanceVBO = 0;
    if (branchInstanceVBO == 0)
        glGenBuffers(1, &branchInstanceVBO);

    glBindBuffer(GL_ARRAY_BUFFER, branchInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 branchModels.size() * sizeof(glm::mat4),
                 branchModels.data(),
                 GL_DYNAMIC_DRAW);

    std::size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4),
                              (void*)(i * vec4Size));
        glVertexAttribDivisor(2 + i, 1);
    }

    branchShader.setInt("useInstancing", 1);
    branchShader.setMat4("model", glm::mat4(1.0f));

    glDrawArraysInstanced(branchMesh.mode, 0, branchMesh.vertexCount,
                          static_cast<GLsizei>(branchModels.size()));

    branchShader.setInt("useInstancing", 0);

    // --------------------------------------------------------
    // Connector caps (non-instanced)
    // --------------------------------------------------------
    for (const Branch& b : branches) {
        if (b.parentIndex < 0 || b.parentIndex >= (int)branches.size())
            continue;

        const Branch& p = branches[b.parentIndex];
        glm::vec3 parentTipWorld =
            glm::vec3(p.worldTransform * glm::vec4(p.localTranslation, 1.0f));

        glm::mat4 capModel =
            worldMatrix *
            glm::translate(glm::mat4(1.0f), parentTipWorld) *
            glm::scale(glm::mat4(1.0f), glm::vec3(capRadius));

        branchShader.setMat4("model", capModel);
        glDrawArrays(branchMesh.mode, 0, branchMesh.vertexCount);
    }

    // --------------------------------------------------------
    // Leaf rendering (instanced)
    // --------------------------------------------------------
    leafShader.use();
    leafShader.setMat4("view", view);
    leafShader.setMat4("projection", projection);

    glBindVertexArray(leafMesh.VAO);

    std::vector<glm::mat4> leafModels;
    leafModels.reserve(leaves.size());

    for (const Leaf& leaf : leaves) {
        if (leaf.parentBranch < 0 || leaf.parentBranch >= (int)branches.size())
            continue;

        const Branch& parent = branches[leaf.parentBranch];

        glm::mat4 leafWorld =
            worldMatrix *
            parent.worldTransform *
            glm::translate(glm::mat4(1.0f), leaf.localOffset) *
            glm::scale(glm::mat4(1.0f), glm::vec3(leaf.size));

        leafModels.push_back(leafWorld);
    }

    static GLuint leafInstanceVBO = 0;
    if (leafInstanceVBO == 0)
        glGenBuffers(1, &leafInstanceVBO);

    glBindBuffer(GL_ARRAY_BUFFER, leafInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 leafModels.size() * sizeof(glm::mat4),
                 leafModels.data(),
                 GL_DYNAMIC_DRAW);

    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4),
                              (void*)(i * vec4Size));
        glVertexAttribDivisor(2 + i, 1);
    }

    leafShader.setInt("useInstancing", 1);
    leafShader.setMat4("model", glm::mat4(1.0f));

    glDrawArraysInstanced(leafMesh.mode, 0, leafMesh.vertexCount,
                          static_cast<GLsizei>(leafModels.size()));

    leafShader.setInt("useInstancing", 0);
}
