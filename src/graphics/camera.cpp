#include "camera.h"
#include "../config.h"
#include "Seb.h"
#include "mesh/mesh_collection.h"
#include <glm/ext/matrix_transform.hpp>

namespace bricksim::graphics {
    void CadCamera::updateVectors() {
        front.x = static_cast<float>(std::cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
        front.y = std::sin(glm::radians(pitch));
        front.z = static_cast<float>(std::sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
        front = glm::normalize(front);

        cameraPos = front * distance + target;
        viewMatrix = glm::lookAt(cameraPos, target, worldUp);
    }

    CadCamera::CadCamera() {
        target = glm::vec3(0.0f, 0.0f, 0.0f);
        mouseRotateSensitivity = config::get(config::MOUSE_3DVIEW_ROTATE_SENSITIVITY);
        mousePanSensitivity = config::get(config::MOUSE_3DVIEW_PAN_SENSITIVITY);
        mouseZoomSensitivity = config::get(config::MOUSE_3DVIEW_ZOOM_SENSITIVITY);
        updateVectors();
    }

    void CadCamera::mouseRotate(float x_delta, float y_delta) {
        yaw += x_delta * mouseRotateSensitivity / 10;
        pitch += y_delta * mouseRotateSensitivity / 10;
        pitch = std::min(89.99f, std::max(-89.99f, pitch));
        updateVectors();
    }

    void CadCamera::mousePan(float x_delta, float y_delta) {
        if (std::abs(x_delta) > 0.01 || std::abs(y_delta) > 0.01) {
            glm::vec3 up{
                    cos(glm::radians(yaw)) * cos(glm::radians(pitch + 90)),
                    sin(glm::radians(pitch + 90)),
                    sin(glm::radians(yaw)) * cos(glm::radians(pitch + 90))};
            glm::vec3 right{-sin(glm::radians(yaw)), 0, cos(glm::radians(yaw))};

            glm::vec3 move = x_delta * mousePanSensitivity / 30 * right + y_delta * mousePanSensitivity / 30 * up;
            target += move;
            cameraPos += move;

            updateVectors();
        }
    }

    void CadCamera::moveForwardBackward(float delta) {
        distance *= 1 - (delta * mouseZoomSensitivity / 10);
        distance = std::max(.1f, distance);
        updateVectors();
    }

    const glm::vec3& CadCamera::getCameraPos() const {
        return cameraPos;
    }

    void CadCamera::setStandardView(int i) {
        switch (i) {
            case 1:
                yaw = 90.0f;
                pitch = 0.0f;
                break;//Front
            case 2:
                yaw = 90.0f;
                pitch = 89.99f;
                break;//Top
            case 3:
                yaw = 0.0f;
                pitch = 0.0f;
                break;//Right
            case 4:
                yaw = 270.0f;
                pitch = 0.0f;
                break;//Rear
            case 5:
                yaw = 90.0f;
                pitch = -89.99f;
                break;//Bottom
            case 6:
                yaw = 180.0f;
                pitch = 0.0f;
                break;//Left
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
    void CadCamera::setTargetPos(const glm::vec3& pos) {
        target = pos;
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

    const glm::mat4& CadCamera::getViewMatrix() const {
        return viewMatrix;
    }

    const glm::vec3& CadCamera::getTargetPos() const {
        return target;
    }

    void CadCamera::mouseRotate(glm::vec2 delta) {
        mouseRotate(delta.x, delta.y);
    }

    void CadCamera::mousePan(glm::vec2 delta) {
        mousePan(delta.x, delta.y);
    }

    aabb::AABB getAABB(const std::shared_ptr<etree::MeshNode>& node) {
        const auto& mesh = mesh::SceneMeshCollection::getMesh(mesh::SceneMeshCollection::getMeshKey(node, false, nullptr), node, nullptr);
        const auto& outerDimensions = mesh->getOuterDimensions();
        auto aabb = outerDimensions.aabb;
        for (const auto& child: node->getChildren()) {
            const auto meshChild = std::dynamic_pointer_cast<etree::MeshNode>(child);
            if (meshChild != nullptr) {
                aabb.includeAABB(getAABB(meshChild));
            }
        }
        return aabb;
    }

    void collectPoints(std::vector<glm::vec3>& points, const std::shared_ptr<etree::MeshNode>& node, const glm::mat4& transformation) {
        const auto& mesh = mesh::SceneMeshCollection::getMesh(mesh::SceneMeshCollection::getMeshKey(node, false, nullptr), node, nullptr);
        const auto& outerDimensions = mesh->getOuterDimensions();
        for (int axis = 0; axis < 3; ++axis) {
            for (int sign = -1; sign < 2; sign += 2) {
                glm::vec4 point(outerDimensions.minEnclosingBallCenter, 1.f);
                point[axis] += outerDimensions.minEnclosingBallRadius * sign;
                points.push_back(point * transformation);
            }
        }
        for (const auto& child: node->getChildren()) {
            const auto meshChild = std::dynamic_pointer_cast<etree::MeshNode>(child);
            if (meshChild != nullptr) {
                collectPoints(points, meshChild, transformation * meshChild->getRelativeTransformation());
            }
        }
    }

    void FitContentCamera::setRootNode(const std::shared_ptr<etree::MeshNode>& node) {
        std::vector<glm::vec3> points;
        collectPoints(points, node, glm::mat4(1.f));
        Seb::Smallest_enclosing_ball<float, glm::vec3> seb(3, points);

        auto center = seb.center_begin();
        const auto sebCenter = glm::vec3(center[0], center[1], center[2]);

        auto meshRadius = seb.radius() * constants::LDU_TO_OPENGL_SCALE;
        target = glm::vec4(sebCenter, 1.0f) * constants::LDU_TO_OPENGL;

        //todo calculate the distance from fov instead of this
        auto distance = meshRadius * 2.45f;
        auto s = glm::radians(45.0f);//todo make variable
        auto t = glm::radians(45.0f);
        cameraPos = glm::vec3(
                            distance * std::cos(s) * std::cos(t),
                            distance * std::sin(s) * std::cos(t),
                            distance * std::sin(t))
                    + target;

        viewMatrix = glm::lookAt(cameraPos,
                                 glm::vec3(0) + target,
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    }

    const glm::mat4& FitContentCamera::getViewMatrix() const {
        return viewMatrix;
    }

    const glm::vec3& FitContentCamera::getCameraPos() const {
        return cameraPos;
    }

    const glm::vec3& FitContentCamera::getTargetPos() const {
        return target;
    }
    Camera::~Camera() = default;
}
