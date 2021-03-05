

#include "gui.h"
#include "../controller.h"
#include "../orientation_cube.h"
#include "gui_internal.h"

namespace gui {
    void windows::drawOrientationCube(bool* show) {
        return;//todo remove this
        ImGui::Begin(WINDOW_NAME_ORIENTATION_CUBE, show);
        ImGui::ImageButton(gui_internal::convertTextureId(orientation_cube::getImage()), ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, 1), 0);
        ImGui::End();
    }
}