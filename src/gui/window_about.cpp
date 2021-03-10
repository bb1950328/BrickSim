

#include "gui.h"
#include "../constant_data/constants.h"
#include "gui_internal.h"
#include "../constant_data/resources.h"

namespace gui {

    namespace {
        struct License {
            const char *const name;
            const char *const hyperlink;
            const char *const usedIn;
            const char *const text;

        };
        const std::vector<License> licenses = {
                License{"Apache License, Version 2.0", "https://www.apache.org/licenses/LICENSE-2.0", "Hugo", (char *) resources::licenses_Apache2_txt},
                License{"2-Clause BSD License", "https://opensource.org/licenses/BSD-2-Clause", "mingw-std-threads", (char *) resources::licenses_bsd2clause_txt},
                License{"3-Clause BSD License", "https://opensource.org/licenses/BSD-3-Clause", "CMake, ", (char *) resources::licenses_bsd3clause_txt},
                License{"GPLv3", "https://www.gnu.org/licenses/gpl-3.0.en.html", "BrickSim itself, Miniball", (char *) resources::licenses_GPLv3_txt},
                License{"MIT", "https://opensource.org/licenses/MIT", "dear imgui, SQLiteCpp, glad", (char *) resources::licenses_MIT_txt},
                License{"zlib", "https://opensource.org/licenses/Zlib", "GLFW, tinyfiledialogs", (char *) resources::licenses_zlib_txt},
        };
    }

    void windows::drawAboutWindow(bool *show) {
        ImGui::Begin(WINDOW_NAME_ABOUT, show, ImGuiWindowFlags_None);
        if (ImGui::BeginTabBar("##aboutTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
            if (ImGui::BeginTabItem("About BrickSim")) {
                ImGui::TextWrapped("BrickSim is a program which should help you build digital brick models.");
                ImGui::TextWrapped(
                        "LEGO(R), the brick configuration, and the minifigure are trademarks of the LEGO Group, which does not sponsor, authorize or endorse this program.");
                ImGui::TextWrapped(
                        "This program comes without any warranty. Neihter the developers nor any other person shall have any liability to any person or entity with respect to any loss or damage caused or alleged to be caused directly or indirectly by this program.");
                ImGui::Separator();
                ImGui::Text("This program is open source. It's source code is available on GitHub under the following link:");
                gui_internal::drawHyperlinkButton("https://www.github.com/bb1950328/BrickSim");
                ImGui::TextWrapped("It's direct contributors have spent %.1f hours for this program.", constants::totalWorkHours);
                ImGui::TextWrapped(
                        "If you got this program from a source which is not listed on GitHub, please uninstall it and report is on GitHub (Create a Issue)");
                ImGui::TextWrapped(
                        "If find a bug, create a issue on GitHub where you describe the steps to reproduce as exact as possible so the developers can fix it.");
                ImGui::TextWrapped("You can also create an issue when you miss a feature or if you have an idea for improvement.");
                ImGui::TextWrapped("If you are a developer, contributing is very appreciated. You will find more information in the README.md");
                ImGui::TextWrapped(
                        "If you have a question or if you think you can contribute in another way (like writing manuals, designing icons or something like that), don't hesistate to create an issue");
                ImGui::Separator();
                ImGui::TextWrapped(
                        "This program wouldn't be possible without the LDraw Parts library. The shapes of all the parts in this programm come from the LDraw project. You will find more information on:");
                gui_internal::drawHyperlinkButton("https://www.ldraw.org");
                ImGui::TextWrapped("The graphical user interface is implemented using Dear ImGUI. More info at: ");
                gui_internal::drawHyperlinkButton("https://github.com/ocornut/imgui");
                ImGui::Separator();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Licenses")) {
                for (const auto &license : licenses) {
                    if (ImGui::CollapsingHeader(license.name)) {
                        ImGui::Text("Libraries under this license: %s", license.usedIn);
                        ImGui::Text("More information available under ");
                        ImGui::SameLine();
                        gui_internal::drawHyperlinkButton(license.hyperlink);

                        if (ImGui::BeginChild(license.name, ImVec2(0.0f, ImGui::GetContentRegionAvail().y-ImGui::GetFontSize()*1.5f))) {
                            ImGui::TextWrapped("%s", license.text);
                            ImGui::EndChild();
                        }
                    }
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        if (ImGui::Button("Close")) {
            *show = false;
        }
        ImGui::End();
    }
}
