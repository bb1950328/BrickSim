#pragma once

#include "../element_tree.h"
#include "../ldr/colors.h"
#include "../ldr/files.h"
#include "../user_actions.h"
#include "windows/windows.h"
#include <glm/glm.hpp>
#include <imgui.h>
#include <memory>
#include <vector>

namespace bricksim {
    class Editor;
}

namespace bricksim::gui_internal {
    /**
     * @return true if the thumbnail is visible, false if it's clipped
     */
    bool drawPartThumbnail(const ImVec2& actualThumbSizeSquared, const std::shared_ptr<ldr::File>& part, ldr::ColorReference color);

    color::RGB getWhiteOrBlackBetterContrast(const glm::vec3& col);

    void drawColorGroup(const std::shared_ptr<etree::MeshNode>& ldrNode, const ImVec2& buttonSize, int columnCount,
                        const std::pair<const std::string, std::vector<ldr::ColorReference>>& colorGroup);

    void drawEditorSelectionCombo(std::weak_ptr<Editor>& selectedEditor, const char* caption = "Editor");

    void drawHyperlinkButton(const std::string& url);

    char getLoFiSpinner();
    const char* getAnimatedHourglassIcon();

    ImTextureID convertTextureId(unsigned int textureId);

    const char* getShortcutText(const user_actions::Action& action);

    void actionMenuItem(const user_actions::Action& action);
    void actionMenuItem(const user_actions::Action& action, const std::shared_ptr<Editor>& editorContext);
    void actionMenuItem(const user_actions::Action& action, const char* alternativeDescription);
    void actionMenuItem(const user_actions::Action& action, const char* alternativeDescription, const std::shared_ptr<Editor>& editorContext);
    void windowMenuItem(gui::windows::Id id);

    template<typename E>
    bool drawEnumCombo(const char* const label, E& value) {
        bool valueChanged = false;
        if (ImGui::BeginCombo(label, magic_enum::enum_name(value).data())) {
            for (const auto& [item, name]: magic_enum::enum_entries<E>()) {
                if (ImGui::Selectable(name.data(), value == item)) {
                    value = item;
                    valueChanged = true;
                }
                if (value == item) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        return valueChanged;
    }
}
