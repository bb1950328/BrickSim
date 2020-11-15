//
// Created by Bader on 15.11.2020.
//

#include "gui.h"
#include "../git_stats.h"
#include "gui_internal.h"

namespace gui {
    void windows::drawAboutWindow(bool *show) {
        if (ImGui::Begin("About", show, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextWrapped("BrickSim is a program which should help you build digital brick models.");
            ImGui::TextWrapped(
                    "LEGO(R), the brick configuration, and the minifigure are trademarks of the LEGO Group, which does not sponsor, authorize or endorse this program.");
            ImGui::TextWrapped(
                    "This program comes without any warranty. Neihter the developers nor any other person shall have any liability to any person or entity with respect to any loss or damage caused or alleged to be caused directly or indirectly by this program.");
            ImGui::Separator();
            ImGui::Text("This program is open source. It's source code is available on GitHub under the following link:");
            gui_internal::draw_hyperlink_button("https://www.github.com/bb1950328/BrickSim");
            ImGui::TextWrapped("It's direct contributors have spent %.1f hours for this program. The following users have contributed:",
                               git_stats::totalHours);
            ImGui::TextWrapped("%s", git_stats::contributorLoc);
            ImGui::Text("The numbers are the lines of code each contributer has committed. (Bigger number doesn't neccessarily mean more effort)");
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
            gui_internal::draw_hyperlink_button("https://www.ldraw.org");
            ImGui::TextWrapped("The graphical user interface is implemented using Dear ImGUI. More info at: ");
            gui_internal::draw_hyperlink_button("https://github.com/ocornut/imgui");
            ImGui::Separator();

            if (ImGui::Button("Close")) {
                *show = false;
            }
            ImGui::End();
        }
    }
}
