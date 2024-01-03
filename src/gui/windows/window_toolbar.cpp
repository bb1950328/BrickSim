#include "window_toolbar.h"
#include "../../config/read.h"
#include "../../editor/tools.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../icons.h"
#include "imgui.h"

namespace bricksim::gui::windows::toolbar {
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse)) {
            const auto buttonSize = ImGui::GetFontSize() * 2.f;
            static const ImVec2 buttonDimensions = {buttonSize, buttonSize};
            static bool horizontal = true;

            for (const auto& tool: magic_enum::enum_values<tools::Tool>()) {
                const auto& toolData = tools::getData(tool);
                const auto active = tools::isActive(tool);
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(active ? ImGuiCol_ButtonActive : ImGuiCol_Button));

                const auto iconCodepoint = icons::getGlyphCodepoint(toolData.icon, icons::Icon48);
                const auto glyph = ImGui::GetFont()->FindGlyph(iconCodepoint);
                if (ImGui::ImageButton(std::string(magic_enum::enum_name(tool)).c_str(),
                                       ImGui::GetFont()->ContainerAtlas->TexID,
                                       buttonDimensions,
                                       {glyph->U0, glyph->V0},
                                       {glyph->U1, glyph->V1})) {
                    tools::setActive(tool);
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay)) {
                    ImGui::SetTooltip("%s", toolData.name.data());
                }
                ImGui::PopStyleColor();
                if (horizontal) {
                    ImGui::SameLine();
                }
            }
            const auto btnLabel = horizontal ? ICON_FA_ELLIPSIS_VERTICAL : ICON_FA_ELLIPSIS;
            const auto btnSize = horizontal
                                     ? ImVec2(25, buttonSize + 2 * ImGui::GetStyle().FramePadding.y)
                                     : ImVec2(buttonSize + 2 * ImGui::GetStyle().FramePadding.x, 25);

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.f, 0.f});
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
            if (ImGui::Button(btnLabel, btnSize)) {
                ImGui::OpenPopup("options");
            }
            ImGui::PopStyleVar(2);
            if (ImGui::BeginPopup("options")) {
                if (ImGui::Selectable(horizontal ? "Vertical" : "Horizontal")) {
                    horizontal = !horizontal;
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
}
