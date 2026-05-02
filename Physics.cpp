#define GLM_ENABLE_EXPERIMENTAL

#include "Physics.h"
#include "Tree.h"
#include "Branch.h"
#include "Leaf.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

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

void Physics::applyCollisions(Tree& tree) const {
    for (Leaf& leaf : tree.leaves) {
        if (leaf.parentBranch < 0 || leaf.parentBranch >= (int)tree.branches.size())
            continue;

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

void Physics::updateBranches(Tree& tree, float dt, float time) const {
    const float globalMaxAngle = glm::radians(30.0f);
    const float globalMaxVel   = glm::radians(120.0f);

    float windNoise =
        0.3f * sin(time * 0.3f) +
        0.15f * sin(time * 1.7f);

    // 1) Integrate physics
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
        float windDrive =
            (windStrength + windNoise) * heightFactor * (1.0f - 0.6f * vertical);

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

    // 2) Recompute world transforms (quaternions)
    for (size_t i = 0; i < tree.branches.size(); ++i) {
        Branch& b = tree.branches[i];

        glm::quat baseQ = glm::quat_cast(b.localRotation);
        glm::quat bendQ = glm::angleAxis(b.bendAngle, glm::vec3(1,0,0));
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
