
#include <iostream>
#include <glm/gtx/normal.hpp>
#include <spdlog/spdlog.h>
#include "camera.h"
#include "../config.h"

void CadCamera::updateVectors() {
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);

    cameraPos = front * distance + target;
    viewMatrix = glm::lookAt(cameraPos, target, worldUp);
}

CadCamera::CadCamera() {
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    mouseRotateSensitivity = (float)config::getDouble(config::MOUSE_3DVIEW_ROTATE_SENSITIVITY);
    mousePanSensitivity = (float)config::getDouble(config::MOUSE_3DVIEW_PAN_SENSITIVITY);
    mouseZoomSensitivity = (float)config::getDouble(config::MOUSE_3DVIEW_ZOOM_SENSITIVITY);
    updateVectors();
}

glm::mat4 CadCamera::getViewMatrix() const {
    return viewMatrix;
}

void CadCamera::mouseRotate(float x_delta, float y_delta) {
    yaw += x_delta * mouseRotateSensitivity / 10;
    pitch += y_delta * mouseRotateSensitivity / 10;
    pitch = std::min(89.99f, std::max(-89.99f, pitch));
    updateVectors();
}

void CadCamera::mousePan(float x_delta, float y_delta) {
    if (std::abs(x_delta)>0.01||std::abs(y_delta)>0.01) {
        glm::vec3 up{
                cos(glm::radians(yaw)) * cos(glm::radians(pitch + 90)),
                sin(glm::radians(pitch+90)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch+90))
        };
        glm::vec3 right{-sin(glm::radians(yaw)),0,cos(glm::radians(yaw))};

        glm::vec3 move = x_delta * mousePanSensitivity / 30 * right + y_delta * mousePanSensitivity / 30 * up;
        target += move;
        cameraPos += move;

        updateVectors();
    }
}

void CadCamera::moveForwardBackward(float delta) {
    distance *= 1-(delta * mouseZoomSensitivity / 10);
    distance = std::max(1.1f, distance);
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
