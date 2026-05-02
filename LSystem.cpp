// LSystem.cpp
#include "LSystem.h"
#include "Branch.h"
#include "Leaf.h"
#include "Utils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace {

// Simple rule application — replace 'F' with a branching pattern.
std::string applyRules(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 2);
    for (char c : input) {
        if (c == 'F') {
            output += "F[+F][-F][&F][^F]F";
        } else {
            output.push_back(c);
        }
    }
    return output;
}

} // namespace

namespace LSystem {

std::string generate(const std::string& axiom, int iterations) {
    std::string current = axiom;
    for (int i = 0; i < iterations; ++i) {
        current = applyRules(current);
    }
    return current;
}

void buildTree(const std::string& seq,
               std::vector<Branch>& branches,
               std::vector<Leaf>& leaves,
               float baseLength,
               float baseRadius)
{
    struct Turtle {
        glm::mat4 orient;
        float length;
        float radius;
        int parentIndex;
        int depth;
    };

    std::vector<Turtle> stack;
    Turtle t;
    t.orient = glm::mat4(1.0f);
    t.length = baseLength;
    t.radius = baseRadius;
    t.parentIndex = -1;
    t.depth = 0;

    for (char c : seq) {
        switch (c) {
            case 'F': {
                float segLength = t.length * Utils::lengthJitter();

                Branch b;
                b.length = segLength;
                b.radius = t.radius;
                b.parentIndex = t.parentIndex;
                b.depth = t.depth;

                // Store rotation only; translation is base->tip in branch-local coords
                b.localRotation = t.orient;
                b.localTranslation = glm::vec3(0.0f, b.length, 0.0f);

                b.stiffness = 8.0f * b.radius;
                b.damping   = 2.0f;
                b.mass      = glm::max(0.001f, b.length);

                int myIndex = static_cast<int>(branches.size());
                branches.push_back(b);

                // --- Compute parent tip first, then apply child rotation
                {
                    Branch& stored = branches[myIndex];
                    glm::mat4 childRot = stored.localRotation;

                    if (stored.parentIndex >= 0 && stored.parentIndex < (int)branches.size()) {
                        const Branch& parent = branches[stored.parentIndex];
                        glm::mat4 parentTip = parent.worldTransform * glm::translate(glm::mat4(1.0f), parent.localTranslation);
                        stored.worldTransform = parentTip * childRot; // child's base at parent tip
                    } else {
                        // root: base at origin, then rotation
                        stored.worldTransform = glm::mat4(1.0f) * childRot;
                    }
                }

                // Optional leaf at tip
                if (t.length < baseLength * 0.9f && (Utils::rng()() % 2 == 0)) {
                    Leaf leaf;
                    leaf.parentBranch = myIndex;
                    leaf.localOffset = glm::vec3(0.0f, branches[myIndex].length, 0.0f);
                    leaf.size = 0.25f * Utils::lengthJitter();
                    leaves.push_back(leaf);
                }

                // Advance turtle
                t.parentIndex = myIndex;
                t.depth += 1;
                t.length *= 0.9f;
                t.radius *= 0.8f;
                if (t.radius < 0.01f) t.radius = 0.01f;
                break;
            }

            case '+':
                t.orient = t.orient * glm::rotate(glm::mat4(1.0f), glm::radians(25.0f + Utils::angleJitter()), glm::vec3(0,1,0));
                break;
            case '-':
                t.orient = t.orient * glm::rotate(glm::mat4(1.0f), glm::radians(-25.0f + Utils::angleJitter()), glm::vec3(0,1,0));
                break;
            case '&':
                t.orient = t.orient * glm::rotate(glm::mat4(1.0f), glm::radians(25.0f + Utils::angleJitter()), glm::vec3(1,0,0));
                break;
            case '^':
                t.orient = t.orient * glm::rotate(glm::mat4(1.0f), glm::radians(-25.0f + Utils::angleJitter()), glm::vec3(1,0,0));
                break;
            case '\\':
                t.orient = t.orient * glm::rotate(glm::mat4(1.0f), glm::radians(25.0f + Utils::angleJitter()), glm::vec3(0,0,1));
                break;
            case '/':
                t.orient = t.orient * glm::rotate(glm::mat4(1.0f), glm::radians(-25.0f + Utils::angleJitter()), glm::vec3(0,0,1));
                break;

            case '[':
                stack.push_back(t);
                break;
            case ']':
                if (!stack.empty()) {
                    t = stack.back();
                    stack.pop_back();
                }
                break;

            default:
                break;
        }
    }

    // Final parent-first recompute using the same correct order (safety)
    for (size_t i = 0; i < branches.size(); ++i) {
        Branch& b = branches[i];
        glm::mat4 childRot = b.localRotation;
        if (b.parentIndex >= 0 && b.parentIndex < (int)branches.size()) {
            const Branch& p = branches[b.parentIndex];
            glm::mat4 parentTip = p.worldTransform * glm::translate(glm::mat4(1.0f), p.localTranslation);
            b.worldTransform = parentTip * childRot;
        } else {
            b.worldTransform = glm::mat4(1.0f) * childRot;
        }
    }
}

} // namespace LSystem
