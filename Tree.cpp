// Tree.cpp
#include "Tree.h"
#include "LSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <iostream>
#include <iomanip>
#include <GLFW/glfw3.h>

void Tree::generate(const std::string& axiom, int iterations,
                    float baseLength, float baseRadius)
{
    branches.clear();
    leaves.clear();

    std::string seq = LSystem::generate(axiom, iterations);
    LSystem::buildTree(seq, branches, leaves, baseLength, baseRadius);

    worldMatrix = glm::mat4(1.0f);

    // Optional connectivity debug (prints only if any gap > threshold)
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

void Tree::draw(const Shader& branchShader,
                const Shader& leafShader,
                const glm::mat4& view,
                const glm::mat4& projection,
                const Physics& physics,
                const Mesh& branchMesh,
                const Mesh& leafMesh) const
{
    // Branch pass
    branchShader.use();
    branchShader.setMat4("view", view);
    branchShader.setMat4("projection", projection);
    branchShader.setVec3("lightDir", glm::normalize(glm::vec3(-0.3f, -1.0f, -0.2f)));
    branchShader.setVec3("lightColor", glm::vec3(1.0f));
    branchShader.setVec3("baseColor", glm::vec3(0.55f, 0.35f, 0.2f));

    glBindVertexArray(branchMesh.VAO);

    // Snap/blend parameters (tune to your scene)
    const float snapThreshold = 0.12f; // max gap to consider for blending/snapping
    const float minSnapEpsilon = 1e-6f;
    const float blendSpeed = 8.0f;     // higher = faster blend toward parent tip
    const float capRadius = 0.035f;    // radius of connector cap drawn at parent tip

    // We'll use a small time-based smoothing factor to avoid popping.
    float time = (float)glfwGetTime();
    // Compute a per-frame blend factor (frame-rate independent if you pass deltaTime).
    // Here we approximate with a simple exponential smoothing using time delta between frames.
    // If you have a global deltaTime, replace dt with that value.
    static float lastTime = 0.0f;
    float dt = (lastTime > 0.0f) ? (time - lastTime) : (1.0f / 60.0f);
    lastTime = time;
    float blendFactor = 1.0f - expf(-blendSpeed * dt); // in (0,1)

    // Assumes branchMesh is a unit cylinder with base at y=0 and top at y=1.
    for (const Branch& b : branches) {
        // base world position (branch base) from stored worldTransform
        glm::vec3 baseWorldPos = glm::vec3(b.worldTransform[3]);

        // If this branch has a parent, compute parent tip and blend toward it if gap small
        if (b.parentIndex >= 0 && b.parentIndex < (int)branches.size()) {
            const Branch& p = branches[b.parentIndex];
            glm::vec3 parentTipWorld = glm::vec3(p.worldTransform * glm::vec4(p.localTranslation, 1.0f));
            float gap = glm::length(baseWorldPos - parentTipWorld);

            if (gap > minSnapEpsilon && gap <= snapThreshold) {
                // Smoothly move the rendered base toward the parent tip to avoid popping
                baseWorldPos = glm::mix(baseWorldPos, parentTipWorld, blendFactor);
            } else if (gap <= minSnapEpsilon) {
                baseWorldPos = parentTipWorld;
            }
        }

        // rotation only from worldTransform
        glm::mat3 rot3 = glm::mat3(b.worldTransform);
        glm::mat4 rotationOnly = glm::mat4(rot3);
        rotationOnly[3] = glm::vec4(0,0,0,1);

        // world placement: tree transform then translate to branch base (snapped/blended)
        glm::mat4 worldPlacement = worldMatrix * glm::translate(glm::mat4(1.0f), baseWorldPos);

        // local geometry: rotate then (if needed translate mesh origin) then scale
        glm::mat4 localGeom = rotationOnly;

        // If your cylinder mesh is centered (-0.5..+0.5), uncomment the next line:
        // localGeom = glm::translate(localGeom, glm::vec3(0.0f, b.length * 0.5f, 0.0f));

        localGeom = glm::scale(localGeom, glm::vec3(b.radius, b.length, b.radius));

        glm::mat4 model = worldPlacement * localGeom;

        branchShader.setMat4("model", model);
        glDrawArrays(branchMesh.mode, 0, branchMesh.vertexCount);
    }

    // Draw small connector caps at parent tips to hide any tiny remaining gaps
    // We reuse the branch shader and branchMesh; draw a small uniformly scaled blob at each parent tip.
    for (const Branch& b : branches) {
        if (b.parentIndex < 0 || b.parentIndex >= (int)branches.size()) continue;
        const Branch& p = branches[b.parentIndex];

        glm::vec3 parentTipWorld = glm::vec3(p.worldTransform * glm::vec4(p.localTranslation, 1.0f));

        // Build a simple model for the cap: place at parent tip, scale uniformly by capRadius
        glm::mat4 capModel = worldMatrix * glm::translate(glm::mat4(1.0f), parentTipWorld);
        capModel = glm::scale(capModel, glm::vec3(capRadius));

        branchShader.setMat4("model", capModel);
        // Draw the same mesh scaled small; it will look like a small cap/plug.
        glDrawArrays(branchMesh.mode, 0, branchMesh.vertexCount);
    }

    // Leaves pass (unchanged)
    leafShader.use();
    leafShader.setMat4("view", view);
    leafShader.setMat4("projection", projection);

    glBindVertexArray(leafMesh.VAO);

    glm::mat4 camRot = view;
    camRot[3][0] = camRot[3][1] = camRot[3][2] = 0.0f;

    time = (float)glfwGetTime();

    for (const Leaf& leaf : leaves) {
        if (leaf.parentBranch < 0 || leaf.parentBranch >= (int)branches.size())
            continue;

        const Branch& parent = branches[leaf.parentBranch];

        glm::vec3 worldPos =
            glm::vec3((worldMatrix * parent.worldTransform) * glm::vec4(leaf.localOffset, 1.0f));

        glm::mat4 model(1.0f);
        model = glm::translate(model, worldPos);

        model *= glm::inverse(camRot);

        float flutter = sin(time * 10.0f + worldPos.x * 5.0f) *
                        0.2f * physics.windStrength;
        model = glm::rotate(model, flutter, glm::vec3(0,1,0));

        float seed = worldPos.x + worldPos.y + worldPos.z;
        float randomRot = fmod(sin(seed * 12.9898f) * 43758.5453f, 6.28318f);
        model = glm::rotate(model, randomRot, glm::vec3(0,1,0));

        model = glm::scale(model, glm::vec3(leaf.size));
        leafShader.setMat4("model", model);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
    }
}
