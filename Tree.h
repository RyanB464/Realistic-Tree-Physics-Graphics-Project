#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Branch.h"
#include "Leaf.h"
#include "Mesh.h"
#include "Shader.h"
#include "Physics.h"

class Tree {
public:
    std::vector<Branch> branches;
    std::vector<Leaf> leaves;

    // Per-tree placement
    glm::vec3 worldPosition = glm::vec3(0.0f);
    glm::mat4 worldMatrix = glm::mat4(1.0f);

    // Generate geometry from an L-system string
    void generate(const std::string& axiom, int iterations,
                  float baseLength, float baseRadius);

    // Draw tree; expects branchMesh to be a cylinder with base at y=0 and height 1.
    void draw(const Shader& branchShader,
              const Shader& leafShader,
              const glm::mat4& view,
              const glm::mat4& projection,
              const Physics& physics,
              const Mesh& branchMesh,
              const Mesh& leafMesh) const;
};
