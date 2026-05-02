#pragma once
#include <glm/glm.hpp>

struct Branch {
    // Local rest orientation (rotation around base)
    glm::mat4 localRotation = glm::mat4(1.0f);

    // Local translation from base to tip in branch-local coordinates (usually (0, length, 0))
    glm::vec3 localTranslation = glm::vec3(0.0f);

    // World transform for the branch base (computed parent-first)
    glm::mat4 worldTransform = glm::mat4(1.0f);

    // Geometry
    float length = 0.0f;
    float radius = 0.0f;

    // Hierarchy
    int parentIndex = -1;
    int depth = 0;

    // Physics state
    float bendAngle = 0.0f;
    float bendVelocity = 0.0f;
    float stiffness = 8.0f;
    float damping = 2.0f;
    float mass = 1.0f;
};
