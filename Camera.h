#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    // Public state
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler angles
    float yaw;
    float pitch;

    // Options
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    // Constructors
    Camera();
    Camera(const glm::vec3& startPos, const glm::vec3& upDir = glm::vec3(0.0f,1.0f,0.0f),
           float startYaw = -90.0f, float startPitch = 0.0f);

    // Input handlers
    void processKeyboard(char key, float deltaTime);
    void processMouse(float xpos, float ypos);

    // View matrix
    glm::mat4 getViewMatrix() const;

private:
    // internal
    void updateCameraVectors();

    // mouse state
    bool firstMouse;
    float lastX;
    float lastY;
};
