// main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STBI_ENABLE_OPENEXR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

// ------------------------------------------------------------
// Texture loader (supports JPG, PNG, EXR)
// ------------------------------------------------------------
unsigned int loadTexture(const char* path)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);

    float* hdrData = nullptr;
    unsigned char* data = nullptr;

    bool isEXR = false;
    std::string p(path);
    if (p.size() > 4 && p.substr(p.size() - 4) == ".exr")
        isEXR = true;

    if (isEXR)
        hdrData = stbi_loadf(path, &width, &height, &channels, 0);
    else
        data = stbi_load(path, &width, &height, &channels, 0);

    if ((!isEXR && !data) || (isEXR && !hdrData)) {
        std::cerr << "Failed to load texture: " << path << "\n";
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) format = GL_RED;
    if (channels == 3) format = GL_RGB;
    if (channels == 4) format = GL_RGBA;

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    if (isEXR) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
                     width, height, 0, format, GL_FLOAT, hdrData);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, format,
                     width, height, 0, format, GL_UNSIGNED_BYTE, data);
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (isEXR) stbi_image_free(hdrData);
    else stbi_image_free(data);

    return tex;
}

// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
Camera gCamera;
float gDeltaTime = 0.0f;
float gLastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    gCamera.processMouse((float)xpos, (float)ypos);
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
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

    // Initial camera position
    gCamera.position = glm::vec3(0.0f, 2.0f, 8.0f);

    glEnable(GL_DEPTH_TEST);

    // --------------------------------------------------------
    // Shaders
    // --------------------------------------------------------
    Shader branchShader("shaders/branch.vert", "shaders/branch.frag");
    Shader leafShader("shaders/leaf.vert", "shaders/leaf.frag");
    Shader groundShader("shaders/ground.vert", "shaders/ground.frag");


    // --------------------------------------------------------
    // Load bark textures
    // --------------------------------------------------------
    unsigned int barkDiffuseID = loadTexture("textures/bark_willow_02_diff_4k.jpg");
    unsigned int barkNormalID  = loadTexture("textures/bark_willow_02_nor_gl_4k.exr");

    branchShader.use();
    branchShader.setInt("barkTex", 0);
    branchShader.setInt("barkNormal", 1);

    // --------------------------------------------------------
    // Shared meshes
    // --------------------------------------------------------
    Mesh branchMesh = createCylinderMesh(24);
    Mesh leafMesh   = createLeafClusterMesh();
    Mesh groundMesh = createGroundPlaneMesh();

    // --------------------------------------------------------
    // Physics system
    // --------------------------------------------------------
    Physics physics;

    // --------------------------------------------------------
    // Create two trees
    // --------------------------------------------------------
    std::vector<Tree> forest;
    forest.reserve(2);

    {
        Tree t;
        t.generate("F", 5, 1.2f, 0.12f);
        t.worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
        t.worldPosition = glm::vec3(-3.0f, 0.0f, 0.0f);

        for (Branch& b : t.branches) {
            b.mass *= 1.0f + 0.2f * (1.0f - glm::clamp((float)b.depth / 6.0f, 0.0f, 1.0f));
            if (b.depth <= 1) b.stiffness *= 2.0f;
        }

        forest.push_back(std::move(t));
    }

    {
        Tree t;
        t.generate("F", 4, 0.9f, 0.10f);
        t.worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
        t.worldPosition = glm::vec3(3.0f, 0.0f, 0.0f);

        for (Branch& b : t.branches) {
            b.mass *= 1.0f + 0.2f * (1.0f - glm::clamp((float)b.depth / 6.0f, 0.0f, 1.0f));
            if (b.depth <= 1) b.stiffness *= 2.0f;
        }

        forest.push_back(std::move(t));
    }

    // --------------------------------------------------------
    // Main loop
    // --------------------------------------------------------
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

        physics.updateColliderFromInput(gDeltaTime);

        float time = (float)glfwGetTime();

        // Physics update
        for (Tree& t : forest) {
            physics.updateBranches(t, gDeltaTime, time);
            physics.applyCollisions(t);
        }

        // Render
        glClearColor(0.18f, 0.24f, 0.28f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = gCamera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.f / 720.f, 0.1f, 200.0f);

        // Bind bark textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, barkDiffuseID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, barkNormalID);

        // Ground (uses its own shader, no bark texture)
        groundShader.use();
        groundShader.setMat4("view", view);
        groundShader.setMat4("projection", projection);

        glm::mat4 groundModel(1.0f);
        groundShader.setMat4("model", groundModel);
        groundShader.setVec3("groundColor", glm::vec3(0.20f, 0.40f, 0.22f));

        glBindVertexArray(groundMesh.VAO);
        glDrawArrays(groundMesh.mode, 0, groundMesh.vertexCount);


        // Trees
        for (const Tree& t : forest) {
            t.draw(branchShader, leafShader, view, projection, physics, branchMesh, leafMesh);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
