#pragma once
#include <glm/glm.hpp>

struct Leaf {
    int parentBranch = -1;          // which branch this leaf belongs to
    glm::vec3 localOffset = {};     // position relative to branch (in branch local space)
    float size = 0.2f;
};
