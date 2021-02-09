

#include "gui.h"
#include "../config.h"
#include "../keyboard_shortcut_manager.h"
#include "../user_actions.h"

namespace gui {
    namespace settings {
        float guiScale;
        int initialWindowSize[2];
        std::string ldrawDirString;
        const char* ldrawDir;
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

        void load() {
            guiScale = (float) (config::getDouble(config::GUI_SCALE));
            initialWindowSize[0] = config::getInt(config::SCREEN_WIDTH);
            initialWindowSize[1] = config::getInt(config::SCREEN_HEIGHT);
            ldrawDirString = config::getString(config::LDRAW_PARTS_LIBRARY);
            ldrawDir = ldrawDirString.c_str();
            guiStyleString = config::getString(config::GUI_STYLE);
            guiStyle = guiStyleString == "light" ? 0 : (guiStyleString == "classic" ? 1 : 2);
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
        }
        void save() {
            config::setDouble(config::GUI_SCALE, guiScale);
            config::setInt(config::SCREEN_WIDTH, initialWindowSize[0]);
            config::setInt(config::SCREEN_HEIGHT, initialWindowSize[1]);
            config::setString(config::LDRAW_PARTS_LIBRARY, ldrawDir);
            switch (guiStyle) {
                case 1:config::setString(config::GUI_STYLE, "ImGuiLight");
                    break;
                case 2:config::setString(config::GUI_STYLE, "ImGuiClassic");
                    break;
                case 3:config::setString(config::GUI_STYLE, "ImGuiDark");
                    break;
                default:config::setString(config::GUI_STYLE, "BrickSim");
                    break;
            }
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
            if (ImGui::BeginTabItem(ICON_FA_KEYBOARD" Shortcuts")) {
                if (ImGui::BeginTable("##key_shortucts", 3)) {
                    for (auto &shortcut : keyboard_shortcut_manager::getAllShortcuts()) {
                        ImGui::TableNextRow();
                        if (ImGui::Button(user_actions::getAction(shortcut.actionId).nameWithIcon)) {
                            //todo action chooser
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::Button(shortcut.getDisplayName().c_str())) {
                            //todo trap next keystroke
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::Button(ICON_FA_TRASH_ALT)) {
                            //todo remove
                        }
                    }
                    ImGui::EndTable();
                }
                if (ImGui::Button(ICON_FA_PLUS" Add new")) {
                    //todo add new
                }
                ImGui::EndTabItem();
            }
        }

        void draw() {
            static bool firstTime = true;
            static float buttonLineHeight = ImGui::GetFontSize()+ImGui::GetStyle().FramePadding.y*5+1;
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
