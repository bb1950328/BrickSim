#include "window_editor_meta_info.h"
#include "../../editor.h"
#include "../gui_internal.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace bricksim::gui::windows::editor_meta_info {
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible, ImGuiWindowFlags_None)) {
            static std::weak_ptr<Editor> selectedEditor;
            static std::weak_ptr<Editor> lastSelectedEditor;
            gui_internal::drawEditorSelectionCombo(selectedEditor, "Model");
            auto selectedEditorLocked = selectedEditor.lock();
            auto& metaInfo = selectedEditorLocked->getEditingModel()->ldrFile->metaInfo;

            ImGui::Separator();

            ImGui::InputText("Name", &metaInfo.name);
            ImGui::InputText("Author", &metaInfo.author);
            ImGui::InputText("License", &metaInfo.license);
            ImGui::InputText("Theme", &metaInfo.theme);

            static std::string keywordsCommaSeparated;
            if (selectedEditorLocked != lastSelectedEditor.lock()) {
                keywordsCommaSeparated = "";
                for (const auto& item: metaInfo.keywords) {
                    keywordsCommaSeparated += item;
                    keywordsCommaSeparated += ", ";
                }
                if (!metaInfo.keywords.empty()) {
                    keywordsCommaSeparated.pop_back();
                    keywordsCommaSeparated.pop_back();
                }
            }
            if (ImGui::InputText("Keywords (comma-separated)", &keywordsCommaSeparated)) {
                metaInfo.keywords.clear();
                metaInfo.addLine("!KEYWORDS" + keywordsCommaSeparated);
            }

            ImGui::Separator();

            static bool explicitCategory;
            if (selectedEditorLocked != lastSelectedEditor.lock()) {
                explicitCategory = metaInfo.headerCategory.has_value();
            }
            if (ImGui::Checkbox("Explicit Category", &explicitCategory)) {
                if (explicitCategory && !metaInfo.headerCategory.has_value()) {
                    metaInfo.headerCategory = metaInfo.getCategory();
                } else if (!explicitCategory && metaInfo.headerCategory.has_value()) {
                    metaInfo.headerCategory = std::nullopt;
                }
            }
            if (explicitCategory) {
                ImGui::InputText("Category", &metaInfo.headerCategory.value());
            } else {
                ImGui::BeginDisabled();
                static char zeroChar = 0;
                ImGui::InputText("Category", &zeroChar, 1);
                ImGui::EndDisabled();
            }

            lastSelectedEditor = selectedEditor;
        }
        ImGui::End();
    }
}
