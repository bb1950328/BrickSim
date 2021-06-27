#include <ostream>
#include <sstream>
#include "../gui.h"
#include "../../helpers/system_info.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome5.h"

#include "window_system_info.h"

namespace bricksim::gui::windows::system_info {
    using namespace helpers::system_info;
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible, ImGuiWindowFlags_AlwaysAutoResize)) {
            static const auto infoLines = getSystemInfo();
            if (ImGui::BeginTable("##sysInfoTable", 2)) {
                for (const auto &line: infoLines) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", line.first.c_str());
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
                data.visible = false;
            }
        }
        ImGui::End();
    }
}
