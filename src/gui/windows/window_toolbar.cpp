#include "window_toolbar.h"
#include "../../config.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../icons.h"
#include "imgui.h"

namespace bricksim::gui::windows::toolbar {
    namespace {
        constexpr float buttonSize = 50.f;
    }
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse)) {
            static const auto scale = config::get(config::GUI_SCALE);
            static const ImVec2 buttonDimensions = {buttonSize * scale, buttonSize * scale};

            static bool horizontal = true;
            for (const auto& [icon, _]: magic_enum::enum_entries<icons::IconType>()) {
                if (ImGui::Button(icons::getGlyph(icon, icons::Icon48), buttonDimensions)) {
                }
                if (horizontal) {
                    ImGui::SameLine();
                }
            }
            const auto btnLabel = horizontal ? ICON_FA_ELLIPSIS_VERTICAL : ICON_FA_ELLIPSIS;
            const auto btnSize = horizontal
                                         ? ImVec2(25 * scale, buttonSize * scale + 2 * ImGui::GetStyle().FramePadding.y)
                                         : ImVec2(buttonSize * scale + 2 * ImGui::GetStyle().FramePadding.x, 25 * scale);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.f, 0.f});
            if (ImGui::Button(btnLabel, btnSize)) {
                ImGui::OpenPopup("options");
            }
            ImGui::PopStyleVar();
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
