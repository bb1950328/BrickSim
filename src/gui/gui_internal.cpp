#include "gui_internal.h"
#include "../info_providers/part_color_availability_provider.h"
#include "../helpers/util.h"
#include "../keyboard_shortcut_manager.h"

namespace gui_internal {
    bool drawPartThumbnail(const ImVec2 &actualThumbSizeSquared, const std::shared_ptr<LdrFile> &part, const LdrColorReference color) {
        bool realThumbnailAvailable = false;
        const bool visible = ImGui::IsRectVisible(actualThumbSizeSquared);
        if (visible) {
            auto optTexId = controller::getThumbnailGenerator()->getThumbnailNonBlocking(part, color);
            if (optTexId.has_value()) {
                auto texId = convertTextureId(optTexId.value());
                ImGui::ImageButton(texId, actualThumbSizeSquared, ImVec2(0, 1), ImVec2(1, 0), 0);
                realThumbnailAvailable = true;
            }
        } else {
            controller::getThumbnailGenerator()->removeFromRenderQueue(part, color);
        }
        if (!realThumbnailAvailable) {
            ImGui::Button(part->metaInfo.name.c_str(), actualThumbSizeSquared);
        }
        if (ImGui::IsItemHovered()) {
            auto availableColors = part_color_availability_provider::getAvailableColorsForPart(part);
            std::string availText;
            if (availableColors.has_value() && !availableColors.value().empty()) {
                if (availableColors.value().size() == 1) {
                    availText = std::string("\nOnly available in ") + availableColors.value().begin()->get()->name;
                } else {
                    availText = std::string("\nAvailable in ") + std::to_string(availableColors.value().size()) + " Colors";
                }
            } else {
                availText = std::string("");
            }
            ImGui::SetTooltip("%s\n%s%s", part->metaInfo.title.c_str(), part->metaInfo.name.c_str(), availText.c_str());
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            controller::insertLdrElement(part);
        }
        return visible;
    }

    ImVec4 getWhiteOrBlackBetterContrast(const glm::vec3 &col) {
        return util::vectorSum(col) > 1.5 ? ImVec4(0, 0, 0, 1) : ImVec4(1, 1, 1, 1);
    }

    void drawColorGroup(const std::shared_ptr<etree::MeshNode> &ldrNode, const ImVec2 &buttonSize, const int columnCount,
                        const std::pair<const std::string, std::vector<LdrColorReference>> &colorGroup) {
        if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            int i = 0;
            for (const auto &color : colorGroup.second) {
                const auto colorValue = color.get();
                if (i % columnCount > 0) {
                    ImGui::SameLine();
                }
                ImGui::PushID(colorValue->code);
                const ImColor imColor = ImColor(colorValue->value.red, colorValue->value.green, colorValue->value.blue);
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) imColor);
                if (ImGui::Button(ldrNode->getDisplayColor().code == color.code ? ICON_FA_CHECK : "", buttonSize)) {
                    ldrNode->setColor(color);
                    controller::setElementTreeChanged(true);
                }
                ImGui::PopStyleColor(/*3*/1);
                ImGui::PopID();
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", color.get()->name.c_str());
                    ImGui::EndTooltip();
                }
                ++i;
            }
            ImGui::TreePop();
        }
    }

    void drawHyperlinkButton(const std::string &url) {
        const auto buttonHoveredColor = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
        ImGui::PushStyleColor(ImGuiCol_Text, buttonHoveredColor);
        ImGui::PushTextWrapPos(-1.0f);
        ImGui::TextUnformatted(url.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
            //todo use this but also set cursor back to arrow when cursor leaves link
            // ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (ImGui::IsMouseClicked(0)) {
                util::openDefaultBrowser(url);
            }
        }
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        min.y = max.y;
        ImGui::GetWindowDrawList()->AddLine(min, max, buttonHoveredColor, 1.0f);
    }

    char getLoFiSpinner() {
        return "|/-\\"[(int) (glfwGetTime() * 8) % 4];
    }

    const char *getAnimatedHourglassIcon() {
        switch ((int) (glfwGetTime() * 6) % 3) {
            case 0:
                return ICON_FA_HOURGLASS_START;
            case 1:
                return ICON_FA_HOURGLASS_HALF;
            case 2:
                return ICON_FA_HOURGLASS_END;
            default:
                return ICON_FA_HOURGLASS;//should never happen
        }
    }

    ImTextureID convertTextureId(unsigned int textureId) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        return (ImTextureID) textureId;
#pragma GCC diagnostic pop
    }

    const char *getShortcutText(const user_actions::Action &action) {
        return keyboard_shortcut_manager::getShortcutForAction(action.id).c_str();
    }

    void actionMenuItem(const user_actions::Action &action) {
        actionMenuItem(action, action.nameWithIcon);
    }

    void actionMenuItem(const user_actions::Action &action, const char *alternativeDescription) {
        if (ImGui::MenuItem(alternativeDescription, gui_internal::getShortcutText(action))) {
            user_actions::executeAction(action.id);
        }
    }
}