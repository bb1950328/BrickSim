

#include "transform_gizmo.h"

#include <utility>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

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
    if (selectedNode != nullptr) {
        auto nodeTransformation = glm::transpose(selectedNode->getAbsoluteTransformation());
        const auto guiScale = config::getDouble(config::GUI_SCALE);

        glm::quat rotation;
        glm::vec3 skewIgnore;
        glm::vec4 perspectiveIgnore;
        glm::vec3 translation;
        glm::vec3 scaleIgnore;
        glm::decompose(nodeTransformation, scaleIgnore, rotation, translation, skewIgnore, perspectiveIgnore);

        glm::vec3 cameraPosLdu = glm::vec4(scene->getCamera()->getCameraPos(), 1.0f) * constants::OPENGL_TO_LDU;

        glm::vec3 direction = glm::normalize(translation - cameraPosLdu);

        glm::vec3 gizmoPos = cameraPosLdu + direction * 5.0f;

        nowTransformation = glm::translate(gizmoPos);
        node->setRelativeTransformation(glm::transpose(nowTransformation.value()));

        node->visible = true;
    } else {
        node->visible = false;
        nowTransformation = {};
    }
    if (nowTransformation != lastTransformation) {
        scene->elementTreeChanged();
        lastTransformation = nowTransformation;
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
