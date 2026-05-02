// main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Tree.h"
#include "Physics.h"
#include "Utils.h"

#include <iostream>
#include <vector>

Camera gCamera; // default-constructed camera
float gDeltaTime = 0.0f;
float gLastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    gCamera.processMouse((float)xpos, (float)ypos);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Tree Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Set initial camera position after GL context is ready
    gCamera.position = glm::vec3(0.0f, 2.0f, 8.0f);

    glEnable(GL_DEPTH_TEST);

    // Shaders
    Shader branchShader("shaders/branch.vert", "shaders/branch.frag");
    Shader leafShader("shaders/leaf.vert", "shaders/leaf.frag");

    // Shared meshes
    Mesh branchMesh = createCylinderMesh(24);      // trunk/branch primitive (base at y=0, height 1)
    Mesh leafMesh   = createLeafClusterMesh();     // leaf cluster primitive
    Mesh groundMesh = createGroundPlaneMesh();     // simple ground plane

    // Physics system
    Physics physics;

    // Create exactly two trees with explicit positions
    std::vector<Tree> forest;
    forest.reserve(2);

    {
        Tree t;
        t.generate("F", 5, 1.2f, 0.12f);   // taller tree
        glm::mat4 world = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
        t.worldMatrix = world;
        t.worldPosition = glm::vec3(-3.0f, 0.0f, 0.0f);

        // Slightly bias trunk stiffness/mass for visual stability
        for (Branch& b : t.branches) {
            b.mass *= 1.0f + 0.2f * (1.0f - glm::clamp((float)b.depth / 6.0f, 0.0f, 1.0f));
            if (b.depth <= 1) b.stiffness *= 2.0f;
        }

        forest.push_back(std::move(t));
    }

    {
        Tree t;
        t.generate("F", 4, 0.9f, 0.10f);   // shorter tree
        glm::mat4 world = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
        t.worldMatrix = world;
        t.worldPosition = glm::vec3(3.0f, 0.0f, 0.0f);

        for (Branch& b : t.branches) {
            b.mass *= 1.0f + 0.2f * (1.0f - glm::clamp((float)b.depth / 6.0f, 0.0f, 1.0f));
            if (b.depth <= 1) b.stiffness *= 2.0f;
        }

        forest.push_back(std::move(t));
    }

    // Print positions for sanity
    for (size_t i = 0; i < forest.size(); ++i) {
        std::cout << "Tree[" << i << "] at " << forest[i].worldPosition.x << ", " << forest[i].worldPosition.z << "\n";
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) gCamera.processKeyboard('W', gDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) gCamera.processKeyboard('S', gDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) gCamera.processKeyboard('A', gDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) gCamera.processKeyboard('D', gDeltaTime);

        // Update collider from input (shared physics controller)
        physics.updateColliderFromInput(gDeltaTime);

        float time = (float)glfwGetTime();

        // Update physics per tree
        for (Tree& t : forest) {
            physics.updateBranches(t, gDeltaTime, time);
            physics.applyCollisions(t);
        }

        // Render
        glClearColor(0.18f, 0.24f, 0.28f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = gCamera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.f / 720.f, 0.1f, 200.0f);

        // Ground pass
        branchShader.use();
        branchShader.setMat4("view", view);
        branchShader.setMat4("projection", projection);
        branchShader.setVec3("lightDir", glm::normalize(glm::vec3(-0.3f, -1.0f, -0.2f)));
        branchShader.setVec3("lightColor", glm::vec3(1.0f));
        branchShader.setVec3("viewPos", gCamera.position);

        // Ground color
        branchShader.setVec3("baseColor", glm::vec3(0.28f, 0.45f, 0.25f));

        glm::mat4 groundModel(1.0f);
        branchShader.setMat4("model", groundModel);
        glBindVertexArray(groundMesh.VAO);
        glDrawArrays(groundMesh.mode, 0, groundMesh.vertexCount);

        // Draw each tree
        for (const Tree& t : forest) {
            t.draw(branchShader, leafShader, view, projection, physics, branchMesh, leafMesh);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
