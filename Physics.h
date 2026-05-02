#pragma once
#include <glm/glm.hpp>

class Tree;

class Physics {
public:
    glm::vec3 windDir;
    float windStrength;

    glm::vec3 colliderPos;
    float colliderRadius;

    Physics();

    void updateColliderFromInput(float deltaTime);
    void applyCollisions(Tree& tree) const;
    void updateBranches(Tree& tree, float deltaTime, float time) const;
};
