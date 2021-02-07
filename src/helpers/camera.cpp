
#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <glm/gtx/normal.hpp>
#include <spdlog/spdlog.h>
#include "camera.h"

void CadCamera::updateVectors() {
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);

    cameraPos = front * distance;
    viewMatrix = glm::lookAt(cameraPos, target, worldUp);
}

CadCamera::CadCamera() {
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    updateVectors();
}

glm::mat4 CadCamera::getViewMatrix() const {
    return viewMatrix;
}

void CadCamera::mouseRotate(float x_delta, float y_delta) {
    yaw += x_delta * mouseMoveSensitivity;
    pitch -= y_delta * mouseMoveSensitivity;
    pitch = std::min(89.99f, std::max(-89.99f, pitch));
    updateVectors();
}

void CadCamera::mousePan(float x_delta, float y_delta) {
    if (std::abs(x_delta)>0.01||std::abs(y_delta)>0.01) {
        //todo is there a more efficient way to do this?? and it doesn't even work :(
        spdlog::debug("mousePan(x_delta={}, y_delta={})", x_delta, y_delta);
        auto translateDown = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1));
        glm::vec3 below_cameraPos = translateDown * glm::vec4(cameraPos, 1.0);
        auto right = glm::triangleNormal(cameraPos, target, below_cameraPos);
        auto translateRight = glm::translate(glm::mat4(1.0f), right);
        glm::vec3 right_of_cameraPos = translateRight * glm::vec4(cameraPos, 1.0);
        auto up = glm::triangleNormal(cameraPos, target, right_of_cameraPos);

        auto panTranslation = glm::translate(glm::mat4(1.0f), right*(x_delta*mousePanSensitivity));
        panTranslation = glm::translate(panTranslation, up*(y_delta*mousePanSensitivity));
        target = panTranslation*glm::vec4(target, 1.0f);
        updateVectors();
    }
}

void CadCamera::moveForwardBackward(float delta) {
    distance-= delta * mouseScrollSensitivity;
    distance = std::max(1.0f, distance);
    updateVectors();
}

const glm::vec3 &CadCamera::getCameraPos() const {
    return cameraPos;
}

void CadCamera::setStandardView(int i) {
    switch (i) {
        case 1: yaw = 90.0f; pitch = 0.0f; break; //Front
        case 2: yaw = 90.0f; pitch = 89.99f; break; //Top
        case 3: yaw = 0.0f; pitch = 0.0f; break; //Right
        case 4: yaw = 270.0f; pitch = 0.0f; break; //Rear
        case 5: yaw = 90.0f; pitch = -89.99f; break; //Bottom
        case 6: yaw = 180.0f; pitch = 0.0f; break; //Left
        default: throw std::invalid_argument("1...6 only");
    }
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    updateVectors();
}

float CadCamera::getDistance() const {
    return distance;
}

void CadCamera::setDistance(float value) {
    distance = value;
    updateVectors();
}

float CadCamera::getPitch() const {
    return pitch;
}

void CadCamera::setPitch(float value) {
    pitch = value;
}

float CadCamera::getYaw() const {
    return yaw;
}

void CadCamera::setYaw(float value) {
    yaw = value;
}
