#include "window_orientation_cube.h"
#include "../../controller.h"
#include "../../graphics/orientation_cube.h"
#include "../gui.h"
#include "../gui_internal.h"

namespace bricksim::gui::windows::orientation_cube {
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

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            const ImVec2& cursorPos = ImGui::GetCursorScreenPos();

            const auto renderedSize = getSize();
            const auto displaySize = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            if (ImGui::ImageButton("orientationCubeImage", gui_internal::convertTextureId(getImage()), ImVec2(displaySize, displaySize), ImVec2(0, 0), ImVec2(1, 1))) {
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
            ImGui::PopStyleVar();

            /*if (config::get(config::DISPLAY_SELECTION_BUFFER)) {
                ImGui::ImageButton(gui_internal::convertTextureId(getSelectionImage()), ImVec2(displaySize, displaySize), ImVec2(0, 0), ImVec2(1, 1), 0);
            }*/
        }
        ImGui::End();
    }
}
