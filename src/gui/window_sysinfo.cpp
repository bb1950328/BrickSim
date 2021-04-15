

#include "gui.h"
#include "../helpers/system_info.h"

namespace gui {
    void windows::drawSysInfoWindow(bool *show) {
        if (ImGui::Begin(WINDOW_NAME_SYSTEM_INFO, show, ImGuiWindowFlags_AlwaysAutoResize)) {
            static const auto infoLines = system_info::getSystemInfo();
            if (ImGui::BeginTable("##sysInfoTable", 2)) {
                for (const auto &line: infoLines) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", line.first);
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", line.second.c_str());
                }
                ImGui::EndTable();
            }

            if (ImGui::Button(ICON_FA_COPY" Copy to clipboard")) {
                std::stringstream result;
                for (const auto &line: infoLines) {
                    result << line.first << "\t" << line.second << std::endl;
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
