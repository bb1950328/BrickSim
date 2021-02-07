

#include "gui.h"

namespace gui {
    void windows::drawSysInfoWindow(bool *show) {
        if (ImGui::Begin(WINDOW_NAME_SYSTEM_INFO, show, ImGuiWindowFlags_AlwaysAutoResize)) {
            static const auto infoLines = util::getSystemInfo();
            for (const auto &line: infoLines) {
                ImGui::Text("%s", line.c_str());
            }
            if (ImGui::Button(ICON_FA_COPY" Copy to clipboard")) {
                std::stringstream result;
                for (const auto &line: infoLines) {
                    result << line << std::endl;
                }
                glfwSetClipboardString(getWindow(), result.str().data());
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_WINDOW_CLOSE" Close")) {
                *show = false;
            }
        }
        ImGui::End();
    }
}
