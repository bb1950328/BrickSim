#include "window_orientation_cube.h"
#include "../../controller.h"
#include "../../graphics/orientation_cube.h"
#include "../gui.h"
#include "../gui_internal.h"

namespace bricksim::gui::windows::orientation_cube {
    using namespace graphics::orientation_cube;
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            const ImVec2& cursorPos = ImGui::GetCursorScreenPos();

            const auto renderedSize = getSize();
            const auto displaySize = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
            if (ImGui::ImageButton(gui_internal::convertTextureId(getImage()), ImVec2(displaySize, displaySize), ImVec2(0, 0), ImVec2(1, 1), 0)) {
                const ImVec2& mousePos = ImGui::GetMousePos();
                const auto scale = (float)renderedSize / displaySize;
                glm::usvec2 imgCoords = {(mousePos.x - cursorPos.x) * scale,
                                         (mousePos.y - cursorPos.y) * scale};
                auto clickedSide = getSide(imgCoords);
                if (clickedSide.has_value()) {
                    switch (clickedSide.value()) {
                        case CubeSide::RIGHT:
                            controller::setStandard3dView(3);
                            break;
                        case CubeSide::BOTTOM:
                            controller::setStandard3dView(5);
                            break;
                        case CubeSide::BACK:
                            controller::setStandard3dView(4);
                            break;
                        case CubeSide::LEFT:
                            controller::setStandard3dView(6);
                            break;
                        case CubeSide::TOP:
                            controller::setStandard3dView(2);
                            break;
                        case CubeSide::FRONT:
                            controller::setStandard3dView(1);
                            break;
                    }
                }
            }

            /*if (config::get(config::DISPLAY_SELECTION_BUFFER)) {
                ImGui::ImageButton(gui_internal::convertTextureId(getSelectionImage()), ImVec2(displaySize, displaySize), ImVec2(0, 0), ImVec2(1, 1), 0);
            }*/
        }
        ImGui::End();
    }
}