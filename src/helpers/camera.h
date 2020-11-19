// camera.h
// Created by bb1950328 on 20.09.20.
//

#ifndef BRICKSIM_CAMERA_H
#define BRICKSIM_CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>


class CadCamera {
public:
    CadCamera();

    [[nodiscard]] float getPitch() const;//degrees
    [[nodiscard]] float getYaw() const;//degrees
    [[nodiscard]] float getDistance() const;
    void setPitch(float value);//degrees
    void setYaw(float value);//degrees
    void setDistance(float value);

    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] const glm::vec3 &getCameraPos() const;

    void mouseRotate(float x_delta, float y_delta);
    void mousePan(float x_delta, float y_delta);
    void moveForwardBackward(float delta);
    void setStandardView(int i);
private:
    float pitch = 0.0f;// up/down angle
    float yaw = 0.0f; // left/right angle
    float distance = 3.0f;// distance between target point and camera

    glm::vec3 front, target, cameraPos;
    glm::vec3 worldUp = glm::vec3(0.0f,  1.0f, 0.0f);
    float mouseMoveSensitivity = 0.1f;
    float mouseScrollSensitivity = 0.33f;
    float mousePanSensitivity = 0.33f;
    glm::mat4 viewMatrix;
    void updateVectors();
};
#endif //BRICKSIM_CAMERA_H
