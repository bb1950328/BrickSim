

#include "transform_gizmo.h"

#include <utility>
#include <algorithm>

const glm::vec4 ARROW_DIRECTIONS[3] = {
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f, 1.0f},
};

TransformGizmo::TransformGizmo(std::shared_ptr<Scene> scene) : scene(std::move(scene)) {}

void TransformGizmo::update() {
    const auto guiScale = config::getDouble(config::GUI_SCALE);
    if (centerDot == nullptr) {
        centerDot = std::make_shared<overlay2d::RegularPolygonElement>(overlay2d::coord_t(0, 0), 5, 10, color::RGB::WHITE);
        scene->getOverlayCollection().addElement(centerDot);

        const static color::RGB ARROW_COLORS[3] = {color::RGB::RED, color::RGB::GREEN, color::RGB::BLUE};
        for (int i = 0; i < 3; ++i) {
            arrows[i] = std::make_shared<overlay2d::ArrowElement>(overlay2d::coord_t(0, 0), overlay2d::coord_t(0, 0), 12 * guiScale, ARROW_COLORS[i]);
            scene->getOverlayCollection().addElement(arrows[i]);
        }
    }

    /*auto selectedNode = etree::getFirstSelectedNode(scene->getRootNode());
    if (selectedNode) {
        selectedNode->getAbsoluteTransformation();
    }*/

    glm::vec3 cameraPos = scene->getCamera()->getCameraPos();
    glm::vec3 targetPos = scene->getCamera()->getTargetPos();
    auto cameraTargetDistance = glm::length(cameraPos - targetPos);
    auto viewMatrix = glm::lookAt(cameraPos - targetPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    auto projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 1000.0f);
    auto projectionView = projectionMatrix * viewMatrix;

    const glm::vec2 center = scene->worldToScreenCoordinates({0.0f, 0.0f, 0.0f});//todo node center here

    for (int i = 0; i < 3; ++i) {
        glm::vec4 diff = projectionView * ARROW_DIRECTIONS[i];
        glm::vec2 diffNDC = diff / diff.w;
        glm::vec2 diffFinal = diffNDC * 200.0f * cameraTargetDistance;
        float length = glm::length(diffFinal);
        auto &arrow = arrows[i];
        if (glm::any(glm::isnan(diffFinal)) || length < 20) {
            arrow->setStart({-100, -100});
            arrow->setEnd({-100, -100});
        } else {
            arrow->setStart(center);
            arrow->setEnd(center + diffFinal);
            arrow->setTipLengthFactor(length / 8.0f / arrow->getLineWidth());
        }
    }
}

TransformGizmo::~TransformGizmo() {
    for (auto & arrow : arrows) {
        scene->getOverlayCollection().removeElement(arrow);
    }
    scene->getOverlayCollection().removeElement(centerDot);
}
