

#include "transform_gizmo.h"

#include <utility>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

const glm::vec4 ARROW_DIRECTIONS[3] = {
        {1.0f, 0.0f,  0.0f,  1.0f},
        {0.0f, 0.0f,  -1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f,  1.0f},
};

TransformGizmo::TransformGizmo(std::shared_ptr<Scene> scene) : scene(std::move(scene)) {
    node = std::make_shared<TransformGizmoNode>(this->scene->getRootNode());
    this->scene->getRootNode()->addChild(node);
    node->initElements();
    for (const auto &arrow : node->getChildren()) {
        arrow->layer = constants::TRANSFORM_GIZMO_LAYER;
    }
}

void TransformGizmo::update() {
    auto selectedNode = getFirstSelectedNode(scene->getRootNode());
    std::optional<glm::mat4> nowTransformation;
    TransformGizmoState nowState;
    if (selectedNode != nullptr) {
        auto nodeTransformation = glm::transpose(selectedNode->getAbsoluteTransformation());
        const static float configScale = config::getDouble(config::GUI_SCALE) * config::getDouble(config::TRANSFORM_GIZMO_SIZE);

        glm::quat rotation;
        glm::vec3 skewIgnore;
        glm::vec4 perspectiveIgnore;
        glm::vec3 translation;
        glm::vec3 scaleIgnore;
        glm::decompose(nodeTransformation, scaleIgnore, rotation, translation, skewIgnore, perspectiveIgnore);

        glm::vec3 cameraPosLdu = glm::vec4(scene->getCamera()->getCameraPos(), 1.0f) * constants::OPENGL_TO_LDU;
        glm::vec3 direction = glm::normalize(translation - cameraPosLdu);
        glm::vec3 gizmoPos = cameraPosLdu + direction * 5.0f * configScale;

        nowTransformation = glm::translate(gizmoPos);
        node->setRelativeTransformation(glm::transpose(nowTransformation.value()));

        float yaw = std::atan2(direction.x, direction.z);
        float pitch = std::atan2(direction.y, direction.y);


        if (yaw <= 0) {
            if (yaw <= -0.5 * M_PI) {
                nowState = pitch < 0
                           ? TransformGizmoState::XPOS_YPOS_ZPOS
                           : TransformGizmoState::XPOS_YPOS_ZNEG;
            } else {
                nowState = pitch < 0
                           ? TransformGizmoState::XPOS_YNEG_ZPOS
                           : TransformGizmoState::XPOS_YNEG_ZNEG;
            }
        } else {
            if (yaw <= 0.5 * M_PI) {
                nowState = pitch < 0
                           ? TransformGizmoState::XNEG_YNEG_ZPOS
                           : TransformGizmoState::XNEG_YNEG_ZNEG;
            } else {
                nowState = pitch < 0
                           ? TransformGizmoState::XNEG_YPOS_ZPOS
                           : TransformGizmoState::XNEG_YPOS_ZNEG;
            }
        }

        node->setState(nowState);

        node->visible = true;
    } else {
        node->visible = false;
        nowTransformation = {};
    }
    if (nowTransformation != lastTransformation || nowState != lastState) {
        scene->elementTreeChanged();
        lastTransformation = nowTransformation;
        lastState = nowState;
    }
}

TransformGizmo::~TransformGizmo() {
    scene->getRootNode()->removeChild(node);
}

bool TransformGizmoNode::isTransformationUserEditable() const {
    return false;
}

bool TransformGizmoNode::isDisplayNameUserEditable() const {
    return false;
}

TransformGizmoNode::TransformGizmoNode(const std::shared_ptr<Node> &parent) : Node(parent) {
    visibleInElementTree = false;
    visible = true;
    displayName = "Transform Gizmo";
}

void TransformGizmoNode::initElements() {
    uint8_t i = 0;
    for (const auto &colorCode : {"#FF0000", "#00FF00", "#0000FF"}) {
        const auto &colorRef = ldr_color_repo::getPureColor(colorCode);
        translateArrows[i] = std::make_shared<generated_mesh::ArrowNode>(colorRef, shared_from_this());
        addChild(translateArrows[i]);

        rotateTori[i] = std::make_shared<generated_mesh::QuarterTorusNode>(colorRef, shared_from_this());
        addChild(rotateTori[i]);

        ++i;
    }
    translateArrows[1]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));
    translateArrows[2]->setRelativeTransformation(glm::rotate(glm::radians(-90.0f), glm::vec3(0, 0, 1)));

    rotateTori[0]->setRelativeTransformation(glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)));
    rotateTori[1]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));
    rotateTori[2]->setRelativeTransformation(glm::rotate(glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)), glm::radians(90.0f), glm::vec3(0, 0, 1)));

    centerBall = std::make_shared<generated_mesh::UVSphereNode>(ldr_color_repo::getPureColor("#ffffff"), shared_from_this());
    centerBall->setRelativeTransformation(glm::scale(glm::vec3(generated_mesh::ArrowNode::LINE_RADIUS * 4)));
    addChild(centerBall);
}

TransformGizmoState TransformGizmoNode::getState() const {
    return state;
}

void TransformGizmoNode::setState(TransformGizmoState newState) {
    if (state != newState) {
        bool newX = (uint8_t) (newState) & (1 << 2);
        bool newY = (uint8_t) (newState) & (1 << 1);
        bool newZ = (uint8_t) (newState) & (1 << 0);

        const auto translationArrowX = glm::translate(glm::vec3(newX ? 0.0f : -1.0f, 0.0f, 0.0f));
        translateArrows[0]->setRelativeTransformation(glm::transpose(translationArrowX));

        const auto rotationArrowY = glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0));
        const auto translationArrowY = glm::translate(glm::vec3(newY ? 0.0f : -1.0f, 0.0f, 0.0f));
        translateArrows[1]->setRelativeTransformation(glm::transpose(rotationArrowY * translationArrowY));

        const auto rotationArrowZ = glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1));
        const auto translationArrowZ = glm::translate(glm::vec3(newZ ? 0.0f : -1.0f, 0.0f, 0.0f));
        translateArrows[2]->setRelativeTransformation(glm::transpose(rotationArrowZ * translationArrowZ));

        float angleTorusX = newY ? (newZ ? -90.0f : 180.0f) : (newZ ? 0.0f : 90.0f);
        rotateTori[0]->setRelativeTransformation(glm::rotate(glm::radians(angleTorusX), glm::vec3(1, 0, 0)));

        float angleTorusY = newX ? (newZ ? 0.0f : 90.0f) : (newZ ? -90.0f : 180.0f);
        const auto rotateTorusY1 = glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0));
        const auto rotateTorusY2 = glm::rotate(glm::radians(angleTorusY), glm::vec3(1, 0, 0));
        rotateTori[1]->setRelativeTransformation(rotateTorusY2 * rotateTorusY1);

        float angleTorusZ = newX ? (newY ? -90.0f : 0.0f) : (newY ? 180.0f : 90.0f);
        const auto rotateTorusZ1 = glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1));
        const auto rotateTorusZ2 = glm::rotate(glm::radians(angleTorusZ), glm::vec3(1, 0, 0));
        rotateTori[2]->setRelativeTransformation(rotateTorusZ2 * rotateTorusZ1);
    }

    state = newState;
}
