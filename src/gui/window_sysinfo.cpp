//
// Created by bb1950328 on 15.11.2020.
//

#include "gui.h"

namespace gui {
    void windows::drawSysInfoWindow(bool *show) {
        if (ImGui::Begin("System info", show, ImGuiWindowFlags_AlwaysAutoResize)) {
            static const auto infoLines = util::getSystemInfo();
            for (const auto &line: infoLines) {
                ImGui::Text("%s", line.c_str());
            }
            if (ImGui::Button("Copy to clipboard")) {
                std::stringstream result;
                for (const auto &line: infoLines) {
                    result << line << std::endl;
                }
                glfwSetClipboardString(getWindow(), result.str().data());
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                *show = false;
            }
        }
        ImGui::End();
    }
}
