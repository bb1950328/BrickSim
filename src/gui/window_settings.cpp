

#include "gui.h"
#include "../config.h"
#include "../keyboard_shortcut_manager.h"
#include "../user_actions.h"

namespace gui {
    namespace settings {
        const char* GUI_STYLE_VALUES[] = {
                "BrickSim",
                "ImGuiLight",
                "ImGuiClassic",
                "ImGuiDark",
        };
        float guiScale;
        int initialWindowSize[2];
        std::string ldrawDirString;
        const char *ldrawDir;
        std::string guiStyleString;
        int guiStyle;
        std::string fontString;
        int font;
        int msaaSamples;
        int msaaElem;
        glm::vec3 backgroundColor;
        glm::vec3 multiPartDocumentColor;
        glm::vec3 mpdSubfileColor;
        glm::vec3 mpdSubfileInstanceColor;
        glm::vec3 officalPartColor;
        glm::vec3 unofficalPartColor;
        bool displaySelectionBuffer;
        bool showNormals;
        std::vector<keyboard_shortcut_manager::KeyboardShortcut> allShortcuts;

        void load() {
            guiScale = (float) (config::getDouble(config::GUI_SCALE));
            initialWindowSize[0] = config::getInt(config::SCREEN_WIDTH);
            initialWindowSize[1] = config::getInt(config::SCREEN_HEIGHT);
            ldrawDirString = config::getString(config::LDRAW_PARTS_LIBRARY);
            ldrawDir = ldrawDirString.c_str();
            guiStyleString = config::getString(config::GUI_STYLE);
            int i=0;
            for (const auto &value : GUI_STYLE_VALUES) {
                if (value==guiStyleString) {
                    guiStyle = i;
                    break;
                }
                ++i;
            }
            fontString = config::getString(config::FONT);
            font = fontString == "Roboto" ? 0 : 1;
            msaaSamples = (int) (config::getInt(config::MSAA_SAMPLES));
            msaaElem = std::log2(msaaSamples);
            backgroundColor = config::getColor(config::BACKGROUND_COLOR).asGlmVector();
            multiPartDocumentColor = config::getColor(config::COLOR_MULTI_PART_DOCUMENT).asGlmVector();
            mpdSubfileColor = config::getColor(config::COLOR_MPD_SUBFILE).asGlmVector();
            mpdSubfileInstanceColor = config::getColor(config::COLOR_MPD_SUBFILE_INSTANCE).asGlmVector();
            officalPartColor = config::getColor(config::COLOR_OFFICAL_PART).asGlmVector();
            unofficalPartColor = config::getColor(config::COLOR_UNOFFICAL_PART).asGlmVector();
            displaySelectionBuffer = config::getBool(config::DISPLAY_SELECTION_BUFFER);
            showNormals = config::getBool(config::SHOW_NORMALS);
            allShortcuts = keyboard_shortcut_manager::getAllShortcuts();
        }

        void save() {
            config::setDouble(config::GUI_SCALE, guiScale);
            config::setInt(config::SCREEN_WIDTH, initialWindowSize[0]);
            config::setInt(config::SCREEN_HEIGHT, initialWindowSize[1]);
            config::setString(config::LDRAW_PARTS_LIBRARY, ldrawDir);
            config::setString(config::GUI_STYLE, GUI_STYLE_VALUES[guiStyle]);
            config::setString(config::FONT, font == 0 ? "Roboto" : "RobotoMono");
            config::setInt(config::MSAA_SAMPLES, (int) std::pow(2, msaaElem));
            config::setColor(config::BACKGROUND_COLOR, util::RGBcolor(backgroundColor));
            config::setColor(config::COLOR_MULTI_PART_DOCUMENT, util::RGBcolor(multiPartDocumentColor));
            config::setColor(config::COLOR_MPD_SUBFILE, util::RGBcolor(mpdSubfileColor));
            config::setColor(config::COLOR_MPD_SUBFILE_INSTANCE, util::RGBcolor(mpdSubfileInstanceColor));
            config::setColor(config::COLOR_OFFICAL_PART, util::RGBcolor(officalPartColor));
            config::setColor(config::COLOR_UNOFFICAL_PART, util::RGBcolor(unofficalPartColor));
            config::setBool(config::DISPLAY_SELECTION_BUFFER, displaySelectionBuffer);
            config::setBool(config::SHOW_NORMALS, showNormals);
            keyboard_shortcut_manager::replaceAllShortcuts(allShortcuts);
        }

        void drawGeneralTab() {
            if (ImGui::BeginTabItem("General")) {
                ImGui::SliderFloat(ICON_FA_EXPAND_ARROWS_ALT" UI Scale", &guiScale, 0.25, 8, "%.2f");
                ImGui::InputInt2(ICON_FA_WINDOW_MAXIMIZE" Initial Window Size", initialWindowSize);
                ImGui::InputText("Ldraw path", const_cast<char *>(ldrawDir), 256);
                ImGui::Combo("GUI Theme", &guiStyle, "BrickSim Default\0ImGui Light\0ImGui Classic\0ImGui Dark\0");
                ImGui::Combo("Font", &font, "Roboto\0Roboto Mono\0");
                ImGui::SliderInt("MSAA Samples", &msaaElem, 0, 4, std::to_string((int) std::pow(2, msaaElem)).c_str());
                ImGui::ColorEdit3(ICON_FA_FILL" Background Color", &backgroundColor.x);
                ImGui::EndTabItem();
            }
        }

        void drawElementTreeTab() {
            if (ImGui::BeginTabItem(ICON_FA_PALETTE" Element Tree")) {
                ImGui::ColorEdit3(ICON_FA_PALETTE" Multi-Part Document Color", &multiPartDocumentColor.x);
                ImGui::ColorEdit3(ICON_FA_PALETTE" MPD Subfile Color", &mpdSubfileColor.x);
                ImGui::ColorEdit3(ICON_FA_PALETTE" MPD Subfile Instance Color", &mpdSubfileInstanceColor.x);
                ImGui::ColorEdit3(ICON_FA_PALETTE" Offical Part Color", &officalPartColor.x);
                ImGui::ColorEdit3(ICON_FA_PALETTE" Unoffical Part Color", &unofficalPartColor.x);
                ImGui::EndTabItem();
            }
        }

        void drawDebugTab() {
            if (ImGui::BeginTabItem(ICON_FA_BUG" Debug")) {
                ImGui::Checkbox(ICON_FA_HAND_POINTER" Display Selection Buffer", &displaySelectionBuffer);
                ImGui::Checkbox(ICON_FA_LONG_ARROW_ALT_UP" Show Normals", &showNormals);
                ImGui::EndTabItem();
            }
        }

        void drawShortcutsTab() {
            static std::optional<std::reference_wrapper<keyboard_shortcut_manager::KeyboardShortcut>> currentlyEditingShortcut;
            bool shouldOpenSelectActionModal = false;
            bool shouldOpenSelectKeyModal = false;
            static bool isWaitingOnKeyCatch = false;
            if (ImGui::BeginTabItem(ICON_FA_KEYBOARD" Shortcuts")) {
                if (ImGui::BeginTable("##key_shortucts", 3)) {
                    for (auto &shortcut : allShortcuts) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        if (ImGui::Button(user_actions::getAction(shortcut.actionId).nameWithIcon)) {
                            currentlyEditingShortcut = shortcut;
                            shouldOpenSelectActionModal = true;
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::Button(shortcut.getDisplayName().c_str())) {
                            currentlyEditingShortcut = shortcut;
                            shouldOpenSelectKeyModal = true;
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::Button(ICON_FA_TRASH_ALT)) {
                            //todo remove
                        }
                    }
                    ImGui::EndTable();
                }
                if (ImGui::Button(ICON_FA_PLUS" Add new")) {
                    allShortcuts.emplace_back(0, 0, 0, keyboard_shortcut_manager::Event::ON_PRESS);
                }
                ImGui::EndTabItem();
            }
            if (shouldOpenSelectActionModal) {
                ImGui::OpenPopup("Select action");
            }
            if (ImGui::BeginPopupModal("Select action", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                constexpr int searchBufSize = 32;
                static char searchBuf[searchBufSize];
                static int currentlySelectedActionId;
                if (shouldOpenSelectActionModal) {
                    currentlySelectedActionId = currentlyEditingShortcut.value().get().actionId;
                }
                ImGui::InputTextWithHint(ICON_FA_SEARCH, "type to filter actions...", searchBuf, searchBufSize);
                if (ImGui::BeginListBox("##actionListBox")) {
                    for (const auto &action : user_actions::findActionsByName(searchBuf)) {
                        const bool is_selected = (action.id == currentlySelectedActionId);
                        if (ImGui::Selectable(action.nameWithIcon, is_selected)) {
                            currentlySelectedActionId = action.id;
                        }

                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndListBox();
                }
                if (ImGui::Button(ICON_FA_CHECK" OK##actionChooser")) {
                    currentlyEditingShortcut.value().get().actionId = currentlySelectedActionId;
                    currentlyEditingShortcut = {};
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_WINDOW_CLOSE" Cancel##actionChooser")) {
                    currentlyEditingShortcut = {};
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            if (shouldOpenSelectKeyModal) {
                keyboard_shortcut_manager::setCatchNextShortcut(true);
                ImGui::OpenPopup("Press keys");
            }
            if (ImGui::BeginPopupModal("Press keys")) {
                ImGui::Text("Press the keys which you want to assign to this action.");
                auto caught = keyboard_shortcut_manager::getCaughtShortcut();
                if (caught.has_value()) {
                    ImGui::Text("Currently selected: %s", caught->getDisplayName().c_str());
                } else {
                    ImGui::Text("Currently selected: %s", currentlyEditingShortcut.value().get().getDisplayName().c_str());
                }

                bool close = false;
                if (ImGui::Button(ICON_FA_CHECK" OK")) {
                    if (caught.has_value()) {
                        currentlyEditingShortcut.value().get().key = caught->key;
                        currentlyEditingShortcut.value().get().modifiers = caught->modifiers;
                    }
                    close = true;
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_WINDOW_CLOSE" Cancel")) {
                    close = true;
                }
                if (close) {
                    isWaitingOnKeyCatch = false;
                    currentlyEditingShortcut = {};
                    keyboard_shortcut_manager::setCatchNextShortcut(false);
                    keyboard_shortcut_manager::clearCaughtShortcut();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        void draw() {
            static bool firstTime = true;
            static float buttonLineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 5 + 1;
            if (firstTime) {
                load();
                firstTime = false;
            }
            if (ImGui::BeginChild("##settingsTabBarWrapper", ImVec2(0, -buttonLineHeight))) {
                if (ImGui::BeginTabBar("##settingsTabBar", ImGuiTabBarFlags_Reorderable)) {
                    drawGeneralTab();
                    drawElementTreeTab();
                    drawDebugTab();
                    drawShortcutsTab();
                    ImGui::EndTabBar();
                }
                ImGui::EndChild();
            }
            ImGui::Separator();
            if (ImGui::Button(ICON_FA_SAVE" Save")) {
                save();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_HISTORY" Discard Changes")) {
                load();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_RETWEET" Restore Defaults")) {
                config::resetAllToDefault();
                load();
            }
        }
    }

    void windows::drawSettingsWindow(bool *show) {
        ImGui::Begin(WINDOW_NAME_SETTINGS, show);
        settings::draw();
        ImGui::End();
    }
}
