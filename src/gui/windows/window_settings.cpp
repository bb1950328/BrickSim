#include "../../config/read.h"
#include "../gui.h"

#include "window_settings.h"

namespace bricksim::gui::windows::settings {
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            ImGui::Text("TODO");
        }
        ImGui::End();
    }
}
