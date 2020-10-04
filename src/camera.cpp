//
// Created by bb1950328 on 24.09.2020.
//

#include <iostream>
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
    //std::cout << "yaw=" << yaw << ", pitch=" << pitch << "\n";
}

void CadCamera::mousePan(float x_delta, float y_delta) {
    //todo implement
}

void CadCamera::moveForwardBackward(float delta) {
    distance-= delta * mouseScrollSensitivity;
    distance = std::max(1.0f, distance);
    updateVectors();
}

const glm::vec3 &CadCamera::getCameraPos() const {
    return cameraPos;
}
