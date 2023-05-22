#include "transform_gizmo2.h"
#include "config.h"
#include "editor.h"

namespace bricksim::transform_gizmo {

    TransformGizmo2::TransformGizmo2(Editor& editor) :
        editor(editor), scene(editor.getScene()) {
        static auto lineWidth = static_cast<o2d::length_t>(4 * config::get(config::GUI_SCALE));
        auto it = axisLines.begin();
        for (const auto color: {color::RED, color::GREEN, color::BLUE}) {
            *it = std::make_shared<o2d::DashedLineElement>(std::vector<o2d::coord_t>{glm::usvec2(0, 0), glm::usvec2(0, 0)}, lineWidth, lineWidth, color);
            scene->getOverlayCollection().addElement(*it);
            ++it;
        }
    }
    void TransformGizmo2::update() {
        glm::vec3 pos = {0, 0, 0};
        constexpr std::array<glm::vec3, 3> axes = {glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)};
        for (int a = 0; a < 3; ++a) {
            std::vector<o2d::coord_t> linePoints;
            linePoints.reserve(21);
            for (int i = -10; i < 11; ++i) {
                const auto p = pos + axes[a] * static_cast<float>(i);
                const auto pScreen = scene->worldToScreenCoordinates(p);
                linePoints.push_back(pScreen);
                
                scene->getOverlayCollection().addElement(std::make_shared<o2d::SquareElement>(pScreen, 16, color::PURPLE));
            }
            axisLines[a]->setPoints(linePoints);
        }
    }
    bool TransformGizmo2::ownsNode(const std::shared_ptr<etree::Node>& node_) {
        return false;
    }
    void TransformGizmo2::startDrag(std::shared_ptr<etree::Node>& draggedNode, glm::svec2 initialCursorPos) {
    }
    void TransformGizmo2::updateCurrentDragDelta(glm::svec2 totalDragDelta) {
    }
    void TransformGizmo2::endDrag() {
    }
    TransformGizmo2::~TransformGizmo2() = default;
}
