//
// Created by Bader on 15.11.2020.
//

#include "gui.h"
#include "../controller.h"
#include "../orientation_cube.h"

namespace gui {
    void windows::drawOrientationCube(bool* show) {
        ImGui::Begin("Orientation Cube", show);
        ImGui::ImageButton((ImTextureID)orientation_cube::getImage(), ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, 1), 0);
        ImGui::End();
    }
}