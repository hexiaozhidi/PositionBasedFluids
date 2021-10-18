#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "shader.h"


enum class CameraMoveDir {
    RIGHT,
    LEFT,
    UP,
    DOWN,
    FORWARD,
    BACKWARD
};

class Camera {
public:
    glm::vec3 position = { 0.0f, 0.0f, 4.0f };
    glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f }; // world-space y-axis, for calculating the right vector of the camera

    float yaw = glm::radians(-90.0f);
    float pitch = 0.0f;

    float fovy = glm::radians(45.0f);
    float tanHalfFovy = tanf(0.5f * fovy);
    float aspect = 16.0f / 9.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;

    Camera(const glm::vec3 &position, float yaw, float pitch, float fovy, float aspect, bool printData = false) : position(position), yaw(yaw), pitch(pitch), fovy(fovy), aspect(aspect), printData(printData) {
        updateAxes();
        updateViewMatrix();
        updateProjectionMatrix();
    }

    Camera(float aspect, bool printData = false) : aspect(aspect), printData(printData) {
        updateAxes();
        updateViewMatrix();
        updateProjectionMatrix();
    }

    void fix() {
        fixed = !fixed;
    }

    const glm::mat4 &getViewMatrix() {
        return view;
    }

    const glm::mat4 &getProjectionMatrix() {
        return projection;
    }

    void processKeyboardMove(CameraMoveDir dir, float deltaTime) {
        if (fixed)
            return;

        float dist = keyboardMoveSpeed * deltaTime;
        switch (dir) {
            case CameraMoveDir::RIGHT:
                position += right * dist;
                break;
            case CameraMoveDir::LEFT:
                position -= right * dist;
                break;
            case CameraMoveDir::UP:
                position += up * dist;
                break;
            case CameraMoveDir::DOWN:
                position -= up * dist;
                break;
            case CameraMoveDir::FORWARD:
                position += front * dist;
                break;
            case CameraMoveDir::BACKWARD:
                position -= front * dist;
                break;
            default:
                break;
        }

        updateViewMatrix();

        if (printData)
            printPosition();
    }

    void processMouseMove(float deltaX, float deltaY, bool constrainPitch = true) {
        if (fixed)
            return;

        yaw += mouseMoveSpeed * deltaX;
        pitch += mouseMoveSpeed * deltaY;
        if (constrainPitch)
            pitch = glm::clamp(pitch, pitchMin, pitchMax);

        updateAxes();
        updateViewMatrix();

        if (printData)
            printEulerAngles();
    }

    void processMouseScroll(float deltaY) {
        if (fixed)
            return;

        fovy -= mouseScrollSpeed * deltaY;
        fovy = glm::clamp(fovy, fovyMin, fovyMax);
        tanHalfFovy = tanf(0.5f * fovy);

        updateProjectionMatrix();

        if (printData)
            printFovy();
    }

    void updateAxes() {
        front = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::cross(right, front);
    }

    void updateViewMatrix() {
        view = glm::lookAt(position, position + front, worldUp);
    }

    void updateProjectionMatrix() {
        projection = glm::perspective(fovy, aspect, zNear, zFar);
    }

    void printPosition() {
        std::cout << "position = (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }

    void printEulerAngles() {
        std::cout << "yaw = " << yaw << ", pitch = " << pitch << std::endl;
    }

    void printFovy() {
        std::cout << "fovy = " << fovy << std::endl;
    }

private:
    bool printData = false;
    bool fixed = false;

    glm::vec3 front; // from the camera to the target
    glm::vec3 up;
    glm::vec3 right;

    glm::mat4 view;
    glm::mat4 projection;

    float pitchMin = glm::radians(-89.0f);
    float pitchMax = glm::radians(89.0f);

    float fovyMin = glm::radians(1.0f);
    float fovyMax = glm::radians(89.0f);

    float keyboardMoveSpeed = 2.5f;
    float mouseMoveSpeed = 0.002f;
    float mouseScrollSpeed = 0.02f;
};
#endif