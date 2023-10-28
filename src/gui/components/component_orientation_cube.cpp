#include "component_orientation_cube.h"

#include "../../config.h"
#include "../../controller.h"
#include "../../graphics/orientation_cube.h"
#include "../gui_internal.h"
#include "imgui.h"

namespace bricksim::gui::components {
    using namespace graphics::orientation_cube;

    int getStandardViewNum(CubeSide cubeSide) {
        switch (cubeSide) {
            case CubeSide::RIGHT: return 3;
            case CubeSide::BOTTOM: return 5;
            case CubeSide::BACK: return 4;
            case CubeSide::LEFT: return 6;
            case CubeSide::TOP: return 2;
            case CubeSide::FRONT: return 1;
            default: return 0;
        }
    }
    void drawOrientationCube(const float displaySize) {
        const ImVec2& cursorPos = ImGui::GetCursorScreenPos();
        const auto renderedSize = getSize();
        const auto textureId = config::get(config::DISPLAY_SELECTION_BUFFER) ? getSelectionImage() : getImage();
        ImGui::Image(gui_internal::convertTextureId(textureId), ImVec2(displaySize, displaySize), ImVec2(0, 0), ImVec2(1, 1));

        static bool lastFrameMouseDown = false;
        bool currentFrameMouseDown = ImGui::IsWindowHovered() && ImGui::GetIO().MouseDown[ImGuiMouseButton_Left];
        if (!currentFrameMouseDown && lastFrameMouseDown) {
            const ImVec2& mousePos = ImGui::GetMousePos();
            const auto scale = (float)renderedSize / displaySize;
            glm::usvec2 imgCoords = {(mousePos.x - cursorPos.x) * scale,
                                     (mousePos.y - cursorPos.y) * scale};
            auto clickedSide = getSide(imgCoords);
            if (clickedSide.has_value()) {
                auto& activeEditor = controller::getActiveEditor();
                if (activeEditor != nullptr) {
                    activeEditor->setStandard3dView(getStandardViewNum(clickedSide.value()));
                }
            }
        }
        lastFrameMouseDown = currentFrameMouseDown;
    }
}
