#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera()
    : position(0.0f, 0.0f, 3.0f),
      front(0.0f, 0.0f, -1.0f),
      worldUp(0.0f, 1.0f, 0.0f),
      yaw(-90.0f),
      pitch(0.0f),
      movementSpeed(5.0f),
      mouseSensitivity(0.1f),
      zoom(45.0f),
      firstMouse(true),
      lastX(400.0f),
      lastY(300.0f)
{
    updateCameraVectors();
}

Camera::Camera(const glm::vec3& startPos, const glm::vec3& upDir, float startYaw, float startPitch)
    : position(startPos),
      front(0.0f, 0.0f, -1.0f),
      worldUp(upDir),
      yaw(startYaw),
      pitch(startPitch),
      movementSpeed(5.0f),
      mouseSensitivity(0.1f),
      zoom(45.0f),
      firstMouse(true),
      lastX(400.0f),
      lastY(300.0f)
{
    updateCameraVectors();
}

void Camera::processKeyboard(char key, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    if (key == 'W') position += front * velocity;
    if (key == 'S') position -= front * velocity;
    if (key == 'A') position -= right * velocity;
    if (key == 'D') position += right * velocity;
}

void Camera::processMouse(float xpos, float ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed: y ranges bottom->top
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

void Camera::updateCameraVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}
