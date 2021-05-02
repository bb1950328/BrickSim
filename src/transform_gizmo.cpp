

#include "transform_gizmo.h"

#include <utility>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>

const glm::vec4 ARROW_DIRECTIONS[3] = {
        {1.0f, 0.0f,  0.0f,  1.0f},
        {0.0f, 0.0f,  -1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f,  1.0f},
};

TransformGizmo::TransformGizmo(std::shared_ptr<Scene> scene) : scene(std::move(scene)) {
    transformGizmoNode = std::make_shared<TransformGizmoNode>(this->scene->getRootNode());
    this->scene->getRootNode()->addChild(transformGizmoNode);
    transformGizmoNode->initElements();
    transformGizmoNode->setRelativeTransformation(glm::scale(glm::vec3(100.0f, 100.0f, 100.0f)));
    for (const auto &arrow : transformGizmoNode->getChildren()) {
        arrow->layer = constants::TRANSFORM_GIZMO_LAYER;
    }
    this->scene->elementTreeChanged();
}

void TransformGizmo::update() {
    const auto guiScale = config::getDouble(config::GUI_SCALE);

    glm::vec3 cameraPos = scene->getCamera()->getCameraPos();
    const glm::vec3 center(0.0f, 0.0f, 0.0f);//todo selected node center here

    auto cameraCenterDistance = glm::length(cameraPos - center);

    //todo update node transformation
}

TransformGizmo::~TransformGizmo() {
    //TODO
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
    for (const auto &color : {"#FF0000", "#00FF00", "#0000FF"}) {
        translateArrows[i] = std::make_shared<generated_mesh::ArrowNode>(ldr_color_repo::getPureColor(color), shared_from_this());
        addChild(translateArrows[i]);
        ++i;
    }
    translateArrows[1]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));
    translateArrows[2]->setRelativeTransformation(glm::rotate(glm::radians(-90.0f), glm::vec3(0, 0, 1)));

    centerBall = std::make_shared<generated_mesh::UVSphereNode>(ldr_color_repo::getPureColor("#cccccc"), shared_from_this());
    centerBall->setRelativeTransformation(glm::scale(glm::vec3(0.4f)));
    addChild(centerBall);
}
