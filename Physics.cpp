#define GLM_ENABLE_EXPERIMENTAL

#include "Physics.h"
#include "Tree.h"
#include "Branch.h"
#include "Leaf.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unordered_map>
#include <vector>
#include <cmath>

// ------------------------------------------------------------
// Simple spatial hash grid for broad-phase leaf collision
// ------------------------------------------------------------
struct GridKey {
    int x, y, z;
    bool operator==(const GridKey& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct GridKeyHasher {
    std::size_t operator()(const GridKey& k) const {
        return ((k.x * 73856093) ^ (k.y * 19349663) ^ (k.z * 83492791));
    }
};

static GridKey makeKey(const glm::vec3& p, float cellSize) {
    return {
        int(floor(p.x / cellSize)),
        int(floor(p.y / cellSize)),
        int(floor(p.z / cellSize))
    };
}

Physics::Physics()
    : windDir(glm::normalize(glm::vec3(1.0f, 0.0f, 0.3f))),
      windStrength(0.6f),
      colliderPos(0.0f, 1.0f, 0.0f),
      colliderRadius(0.5f)
{}

void Physics::updateColliderFromInput(float dt) {
    GLFWwindow* w = glfwGetCurrentContext();
    if (!w) return;

    float speed = 2.0f * dt;

    if (glfwGetKey(w, GLFW_KEY_I) == GLFW_PRESS) colliderPos.z -= speed;
    if (glfwGetKey(w, GLFW_KEY_K) == GLFW_PRESS) colliderPos.z += speed;
    if (glfwGetKey(w, GLFW_KEY_J) == GLFW_PRESS) colliderPos.x -= speed;
    if (glfwGetKey(w, GLFW_KEY_L) == GLFW_PRESS) colliderPos.x += speed;
}

// ------------------------------------------------------------
// Broad-phase + narrow-phase leaf collisions (spatial grid)
// ------------------------------------------------------------
void Physics::applyCollisions(Tree& tree) const {
    const float cellSize = 1.0f;

    std::unordered_map<GridKey, std::vector<int>, GridKeyHasher> grid;
    grid.reserve(tree.leaves.size());

    // Build spatial grid
    for (int i = 0; i < (int)tree.leaves.size(); ++i) {
        const Leaf& leaf = tree.leaves[i];
        if (leaf.parentBranch < 0 || leaf.parentBranch >= (int)tree.branches.size())
            continue;

        const Branch& parent = tree.branches[leaf.parentBranch];
        glm::vec3 worldPos =
            glm::vec3(parent.worldTransform * glm::vec4(leaf.localOffset, 1.0f));

        GridKey key = makeKey(worldPos, cellSize);
        grid[key].push_back(i);
    }

    // Query only nearby cells
    GridKey c = makeKey(colliderPos, cellSize);

    for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
    for (int dz = -1; dz <= 1; ++dz)
    {
        GridKey key{ c.x + dx, c.y + dy, c.z + dz };

        auto it = grid.find(key);
        if (it == grid.end()) continue;

        for (int leafIndex : it->second) {
            Leaf& leaf = tree.leaves[leafIndex];
            const Branch& parent = tree.branches[leaf.parentBranch];

            glm::vec3 worldPos =
                glm::vec3(parent.worldTransform * glm::vec4(leaf.localOffset, 1.0f));

            glm::vec3 d = worldPos - colliderPos;
            float dist = glm::length(d);

            if (dist < colliderRadius && dist > 0.0001f) {
                glm::vec3 push = glm::normalize(d) * (colliderRadius - dist);

                glm::mat4 invParent = glm::inverse(parent.worldTransform);
                glm::vec3 localPush =
                    glm::vec3(invParent * glm::vec4(push, 0.0f));

                leaf.localOffset += localPush * 0.5f;
            }
        }
    }
}

// ------------------------------------------------------------
// Branch physics (multi-axis, wind-direction-aware bending)
// ------------------------------------------------------------
void Physics::updateBranches(Tree& tree, float dt, float time) const {
    const float globalMaxAngle = glm::radians(30.0f);
    const float globalMaxVel   = glm::radians(120.0f);

    float windNoise =
        0.3f * sin(time * 0.3f) +
        0.15f * sin(time * 1.7f);

    // 1) Integrate scalar bend angle (magnitude), but axis will be directional
    for (size_t i = 0; i < tree.branches.size(); ++i) {
        Branch& b = tree.branches[i];

        if (i == 0) {
            b.bendAngle = 0.0f;
            b.bendVelocity = 0.0f;
            continue;
        }

        glm::vec3 localUp =
            glm::normalize(glm::vec3(b.localRotation * glm::vec4(0,1,0,0)));
        float vertical =
            glm::clamp(glm::dot(localUp, glm::vec3(0,1,0)), 0.0f, 1.0f);

        float depthScale =
            glm::mix(1.0f, 6.0f, glm::clamp(1.0f - b.depth * 0.15f, 0.0f, 1.0f));
        float verticalScale =
            glm::mix(1.0f, 10.0f, vertical);

        float k = b.stiffness * depthScale * verticalScale;
        float c = b.damping * glm::mix(1.0f, 2.5f, vertical);

        float heightFactor = glm::clamp(b.length * 0.5f, 0.1f, 2.0f);

        // Project wind onto plane perpendicular to localUp
        glm::vec3 w = glm::normalize(windDir);
        glm::vec3 wProj = w - glm::dot(w, localUp) * localUp;
        float lateralFactor = glm::length(wProj); // 0 if aligned with up

        float windDrive =
            (windStrength + windNoise) * heightFactor *
            (0.3f + 0.7f * lateralFactor) *
            (1.0f - 0.6f * vertical);

        float targetAngle =
            windDrive * 0.25f * sin(time * (1.0f + b.depth * 0.2f));

        float gravityBias = 0.15f * sin(b.bendAngle);

        float parentCoupling = 0.0f;
        if (b.parentIndex >= 0) {
            const Branch& p = tree.branches[b.parentIndex];
            glm::vec3 parentUp =
                glm::normalize(glm::vec3(p.worldTransform * glm::vec4(0,1,0,0)));
            glm::vec3 myUp =
                glm::normalize(glm::vec3(b.worldTransform * glm::vec4(0,1,0,0)));

            float rel = glm::dot(parentUp, myUp);
            parentCoupling = (1.0f - glm::clamp(rel, -1.0f, 1.0f)) * 0.4f * k;
        }

        float torque =
            k * (targetAngle - b.bendAngle)
            - c * b.bendVelocity
            - parentCoupling * b.bendAngle
            - gravityBias;

        float acc = torque / glm::max(b.mass, 0.001f);

        b.bendVelocity += acc * dt;
        b.bendVelocity =
            glm::clamp(b.bendVelocity, -globalMaxVel, globalMaxVel);

        b.bendAngle += b.bendVelocity * dt;

        float localMax =
            glm::mix(glm::radians(8.0f), globalMaxAngle,
                     glm::clamp(b.depth * 0.25f + (1.0f - vertical),
                                0.0f, 1.0f));

        if (fabs(b.bendAngle) > localMax) {
            b.bendAngle = glm::clamp(b.bendAngle, -localMax, localMax);
            b.bendVelocity *= 0.2f;
        }
    }

    // 2) Recompute world transforms with directional bend axis
    for (size_t i = 0; i < tree.branches.size(); ++i) {
        Branch& b = tree.branches[i];

        glm::quat baseQ = glm::quat_cast(b.localRotation);

        glm::vec3 localUp =
            glm::normalize(glm::vec3(b.localRotation * glm::vec4(0,1,0,0)));

        glm::vec3 w = glm::normalize(windDir);
        glm::vec3 bendAxis = glm::cross(localUp, w);

        float axisLen = glm::length(bendAxis);
        if (axisLen < 1e-4f) {
            bendAxis = glm::normalize(glm::cross(localUp, glm::vec3(1,0,0)));
            if (glm::length(bendAxis) < 1e-4f)
                bendAxis = glm::vec3(0,0,1);
        } else {
            bendAxis /= axisLen;
        }

        glm::quat bendQ = glm::angleAxis(b.bendAngle, bendAxis);
        glm::quat finalQ = baseQ * bendQ;

        glm::mat4 rotMat = glm::mat4_cast(finalQ);

        glm::mat4 local =
            rotMat *
            glm::translate(glm::mat4(1.0f), b.localTranslation);

        if (b.parentIndex >= 0)
            b.worldTransform = tree.branches[b.parentIndex].worldTransform * local;
        else
            b.worldTransform = local;
    }
}
