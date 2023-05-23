#include "transform_gizmo2.h"
#include "config.h"
#include "controller.h"
#include "editor.h"
#include "glm/gtx/string_cast.hpp"
#include "spdlog/spdlog.h"

namespace bricksim::transform_gizmo {

    TransformGizmo2::TransformGizmo2(Editor& editor) :
        editor(editor), scene(editor.getScene()) {
        static auto lineWidth = static_cast<o2d::length_t>(8 * config::get(config::GUI_SCALE));
        auto it = axisLines.begin();
        for (const auto color: {color::RED, color::GREEN, color::BLUE}) {
            *it = std::make_shared<o2d::DashedLineElement>(std::vector<o2d::coord_t>{o2d::coord_t(0, 0), o2d::coord_t(0, 0)}, lineWidth, lineWidth, color);
            scene->getOverlayCollection().addElement(*it);
            ++it;
        }
    }
    void TransformGizmo2::update() {
        const auto& linearSnapPreset = controller::getSnapHandler().getLinear().getCurrentPreset();
        glm::vec3 pos = {0, 0, 0};
        constexpr std::array<glm::vec3, 3> axes = {glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)};
        for (int a = 0; a < 3; ++a) {
            std::vector<o2d::coord_t> linePoints;
            const auto step = a == 1 ? linearSnapPreset.stepY : linearSnapPreset.stepXZ;
            for (int i = -10; i < 11; ++i) {
                const glm::vec3 worldCoord = pos + axes[a] * static_cast<float>(i * step);
                const glm::vec3 screenCoord = scene->worldToScreenCoordinates(glm::vec4(worldCoord, 1.f) * constants::LDU_TO_OPENGL);
                if (-1 <= screenCoord.z && screenCoord.z <= 1) {
                    linePoints.emplace_back(screenCoord);
                }
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
