#include "window_orientation_cube.h"
#include "../components/component_orientation_cube.h"
#include "../gui.h"

namespace bricksim::gui::windows::orientation_cube {
    void draw(Data& data) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            const auto displaySize = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
            components::drawOrientationCube(displaySize);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}
