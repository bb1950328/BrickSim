

#include "gui.h"
#include "../controller.h"
#include "../orientation_cube.h"
#include "gui_internal.h"
#include "../config.h"

namespace gui {
    void windows::drawOrientationCube(bool *show) {
        ImGui::Begin(WINDOW_NAME_ORIENTATION_CUBE, show);
        const ImVec2 &cursorPos = ImGui::GetCursorScreenPos();

        const static auto size = ImVec2(orientation_cube::getSize(), orientation_cube::getSize());
        if (ImGui::ImageButton(gui_internal::convertTextureId(orientation_cube::getImage()), size, ImVec2(0, 0), ImVec2(1, 1), 0)) {
            const ImVec2 &mousePos = ImGui::GetMousePos();
            glm::usvec2 imgCoords = {mousePos.x - cursorPos.x, mousePos.y - cursorPos.y};
            auto clickedSide = orientation_cube::getSide(imgCoords);
            if (clickedSide.has_value()) {
                switch (clickedSide.value()) {
                    case orientation_cube::CubeSide::RIGHT:
                        controller::setStandard3dView(3);
                        break;
                    case orientation_cube::CubeSide::BOTTOM:
                        controller::setStandard3dView(5);
                        break;
                    case orientation_cube::CubeSide::BACK:
                        controller::setStandard3dView(4);
                        break;
                    case orientation_cube::CubeSide::LEFT:
                        controller::setStandard3dView(6);
                        break;
                    case orientation_cube::CubeSide::TOP:
                        controller::setStandard3dView(2);
                        break;
                    case orientation_cube::CubeSide::FRONT:
                        controller::setStandard3dView(1);
                        break;
                }
            }
        }

        if (config::getBool(config::DISPLAY_SELECTION_BUFFER) || true) {
            ImGui::ImageButton(gui_internal::convertTextureId(orientation_cube::getSelectionImage()), size, ImVec2(0, 0), ImVec2(1, 1), 0);
        }

        ImGui::End();
    }
}