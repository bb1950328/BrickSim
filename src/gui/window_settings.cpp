//
// Created by Bader on 15.11.2020.
//

#include "gui.h"
#include "../config.h"

namespace gui {
    void windows::drawSettingsWindow(bool *show) {
        ImGui::Begin("Settings", show);
        static auto guiScale = (float) (config::getDouble(config::GUI_SCALE));
        static int initialWindowSize[2]{
                static_cast<int>(config::getInt(config::SCREEN_WIDTH)),
                static_cast<int>(config::getInt(config::SCREEN_HEIGHT))
        };
        static auto ldrawDirString = config::getString(config::LDRAW_PARTS_LIBRARY);
        static auto ldrawDir = ldrawDirString.c_str();
        static auto guiStyleString = config::getString(config::GUI_STYLE);
        static auto guiStyle = guiStyleString == "light" ? 0 : (guiStyleString == "classic" ? 1 : 2);
        static int msaaSamples = (int) (config::getInt(config::MSAA_SAMPLES));
        static int msaaElem = std::log2(msaaSamples);
        static glm::vec3 backgroundColor = config::getColor(config::BACKGROUND_COLOR).asGlmVector();
        static glm::vec3 multiPartDocumentColor = config::getColor(config::COLOR_MULTI_PART_DOCUMENT).asGlmVector();
        static glm::vec3 mpdSubfileColor = config::getColor(config::COLOR_MPD_SUBFILE).asGlmVector();
        static glm::vec3 mpdSubfileInstanceColor = config::getColor(config::COLOR_MPD_SUBFILE_INSTANCE).asGlmVector();
        static glm::vec3 officalPartColor = config::getColor(config::COLOR_OFFICAL_PART).asGlmVector();
        static glm::vec3 unofficalPartColor = config::getColor(config::COLOR_UNOFFICAL_PART).asGlmVector();
        static bool displaySelectionBuffer = config::getBool(config::DISPLAY_SELECTION_BUFFER);
        static bool showNormals = config::getBool(config::SHOW_NORMALS);
        if (ImGui::TreeNode("General Settings")) {
            ImGui::SliderFloat("UI Scale", &guiScale, 0.25, 8, "%.2f");
            ImGui::InputInt2("Initial Window Size", initialWindowSize);
            ImGui::InputText("Ldraw path", const_cast<char *>(ldrawDir), 256);
            ImGui::Combo("GUI Theme", &guiStyle, "Light\0Classic\0Dark\0");
            ImGui::SliderInt("MSAA Samples", &msaaElem, 0, 4, std::to_string((int) std::pow(2, msaaElem)).c_str());
            ImGui::ColorEdit3("Background Color", &backgroundColor.x);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Element Tree Settings")) {
            ImGui::ColorEdit3("Multi-Part Document Color", &multiPartDocumentColor.x);
            ImGui::ColorEdit3("MPD Subfile Color", &mpdSubfileColor.x);
            ImGui::ColorEdit3("MPD Subfile Instance Color", &mpdSubfileInstanceColor.x);
            ImGui::ColorEdit3("Offical Part Color", &officalPartColor.x);
            ImGui::ColorEdit3("Unoffical Part Color", &unofficalPartColor.x);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Debug Settings")) {
            ImGui::Checkbox("Display Selection Buffer", &displaySelectionBuffer);
            ImGui::Checkbox("Show Normals", &showNormals);
            ImGui::TreePop();
        }
        static bool saveFailed = false;
        if (ImGui::Button("Save")) {
            config::setDouble(config::GUI_SCALE, guiScale);
            config::setInt(config::SCREEN_WIDTH, initialWindowSize[0]);
            config::setInt(config::SCREEN_HEIGHT, initialWindowSize[1]);
            config::setString(config::LDRAW_PARTS_LIBRARY, ldrawDir);
            switch (guiStyle) {
                case 0:
                    config::setString(config::GUI_STYLE, "light");
                    break;
                case 1:
                    config::setString(config::GUI_STYLE, "classic");
                    break;
                default:
                    config::setString(config::GUI_STYLE, "dark");
                    break;
            }
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
        ImGui::End();
    }
}
