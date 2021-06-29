#pragma once

#include "../element_tree.h"
#include <glm/glm.hpp>
#include <memory>

namespace bricksim::graphics {

    class Camera {
    public:
        [[nodiscard]] virtual const glm::mat4& getViewMatrix() const = 0;
        [[nodiscard]] virtual const glm::vec3& getCameraPos() const = 0;
        [[nodiscard]] virtual const glm::vec3& getTargetPos() const = 0;
    };

    class CadCamera : public Camera {
    public:
        CadCamera();

        [[nodiscard]] float getPitch() const;//degrees
        [[nodiscard]] float getYaw() const;  //degrees
        [[nodiscard]] float getDistance() const;
        void setPitch(float value);//degrees
        void setYaw(float value);  //degrees
        void setDistance(float value);

        [[nodiscard]] const glm::mat4& getViewMatrix() const override;
        [[nodiscard]] const glm::vec3& getCameraPos() const override;
        [[nodiscard]] const glm::vec3& getTargetPos() const override;

        void mouseRotate(float x_delta, float y_delta);
        void mouseRotate(glm::vec2 delta);
        void mousePan(float x_delta, float y_delta);
        void mousePan(glm::vec2 delta);
        void moveForwardBackward(float delta);
        void setStandardView(int i);

    private:
        float pitch = 0.0f;   // up/down angle
        float yaw = 0.0f;     // left/right angle
        float distance = 3.0f;// distance between target point and camera

        glm::vec3 front, target, cameraPos;
        const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        float mouseRotateSensitivity;
        float mouseZoomSensitivity;
        float mousePanSensitivity;
        glm::mat4 viewMatrix;
        void updateVectors();
    };

    class FitContentCamera : public Camera {
    public:
        void setRootNode(const std::shared_ptr<etree::MeshNode>& node);
        [[nodiscard]] const glm::mat4& getViewMatrix() const override;
        [[nodiscard]] const glm::vec3& getCameraPos() const override;
        [[nodiscard]] const glm::vec3& getTargetPos() const override;

    private:
        glm::mat4 viewMatrix;
        glm::vec3 cameraPos, target;
    };
}
