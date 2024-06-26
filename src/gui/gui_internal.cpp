#include "gui_internal.h"
#include "../controller.h"
#include "../helpers/util.h"
#include "../info_providers/part_color_availability_provider.h"
#include "../keyboard_shortcut_manager.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome6.h"

namespace bricksim::gui_internal {
    bool drawPartThumbnail(const ImVec2& actualThumbSizeSquared, const std::shared_ptr<ldr::File>& part, const ldr::ColorReference color) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(config::get().graphics.background));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
        bool realThumbnailDrawn = false;
        const bool visible = ImGui::IsRectVisible(actualThumbSizeSquared);
        if (visible) {
            auto optTexId = controller::getThumbnailGenerator()->getThumbnailNonBlocking({part, color});
            if (optTexId.has_value()) {
                auto texId = convertTextureId(optTexId.value()->getID());
                ImGui::ImageButton(part->metaInfo.name.c_str(), texId, actualThumbSizeSquared, ImVec2(0, 1), ImVec2(1, 0));
                realThumbnailDrawn = true;
            }
        } else {
            controller::getThumbnailGenerator()->removeFromRenderQueue({part, color});
        }
        if (!realThumbnailDrawn) {
            ImGui::Button(part->metaInfo.name.c_str(), actualThumbSizeSquared);
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
            auto availableColors = info_providers::part_color_availability::getAvailableColorsForPart(part);
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
            const auto& activeEditor = controller::getActiveEditor();
            if (activeEditor != nullptr) {
                activeEditor->insertLdrElement(part);
            }
        }
        return visible;
    }

    color::RGB getWhiteOrBlackBetterContrast(const glm::vec3& col) {
        return util::vectorSum(col) > 1.5 ? color::BLACK : color::WHITE;
    }

    void drawColorGroup(const std::shared_ptr<etree::MeshNode>& ldrNode, const ImVec2& buttonSize, const int columnCount,
                        const std::pair<const std::string, std::vector<ldr::ColorReference>>& colorGroup) {
        if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            int i = 0;
            for (const auto& color: colorGroup.second) {
                const auto colorValue = color.get();
                if (i % columnCount > 0) {
                    ImGui::SameLine();
                }
                ImGui::PushID(colorValue->code);
                ImGui::PushStyleColor(ImGuiCol_Button, colorValue->value);
                if (ImGui::Button(ldrNode->getDisplayColor().code == color.code ? ICON_FA_CHECK : "", buttonSize)) {
                    ldrNode->setColor(color);
                    ldrNode->incrementVersion();
                }
                ImGui::PopStyleColor(/*3*/ 1);
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

    void drawHyperlinkButton(const std::string& url) {
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
        return "|/-\\"[(int)(glfwGetTime() * 8) % 4];
    }

    const char* getAnimatedHourglassIcon() {
        switch ((int)(glfwGetTime() * 6) % 3) {
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
        return (ImTextureID)textureId;
        #pragma GCC diagnostic pop
    }

    const char* getShortcutText(const user_actions::Action& action) {
        return keyboard_shortcut_manager::getShortcutForAction(action).c_str();
    }

    void actionMenuItem(const user_actions::Action& action) {
        actionMenuItem(action, std::shared_ptr<Editor>(nullptr));
    }

    void actionMenuItem(const user_actions::Action& action, const std::shared_ptr<Editor>& editorContext) {
        actionMenuItem(action, user_actions::getName(action).data(), editorContext);
    }

    void actionMenuItem(const user_actions::Action& action, const char* alternativeDescription) {
        actionMenuItem(action, alternativeDescription, nullptr);
    }

    void actionMenuItem(const user_actions::Action& action, const char* alternativeDescription, const std::shared_ptr<Editor>& editorContext) {
        if (ImGui::MenuItem(alternativeDescription, gui_internal::getShortcutText(action), false, user_actions::isEnabled(action))) {
            user_actions::execute(action, editorContext);
        }
    }

    void windowMenuItem(const gui::windows::Id id) {
        ImGui::MenuItem(gui::windows::getName(id), "", gui::windows::isVisible(id));
    }

    void drawEditorSelectionCombo(std::weak_ptr<Editor>& selectedEditor, const char* const caption) {
        auto lockedSelected = selectedEditor.lock();
        if (lockedSelected == nullptr) {
            lockedSelected = *controller::getEditors().begin();
            selectedEditor = lockedSelected;
        }
        if (ImGui::BeginCombo(caption, lockedSelected->getFilename().c_str())) {
            for (const auto& editor: controller::getEditors()) {
                const bool isSelected = lockedSelected == editor;
                if (ImGui::Selectable(editor->getFilename().c_str(), isSelected)) {
                    selectedEditor = editor;
                    lockedSelected = editor;
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
}
