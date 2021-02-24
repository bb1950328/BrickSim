

#include "gui.h"
#include "../constant_data/git_stats.h"
#include "gui_internal.h"
#include "../tools/gears.h"

namespace gui {
    void windows::drawGearRatioCalculatorWindow(bool *show) {
        if (ImGui::Begin(WINDOW_NAME_GEAR_RATIO_CALCULATOR, show, ImGuiWindowFlags_AlwaysAutoResize)) {
            static std::list<gears::GearPair> pairs = {gears::GearPair(gears::GEAR_8T, gears::GEAR_8T)};
            auto totalRatio = Fraction(1, 1);
            if (ImGui::BeginTable("##gearPairs", 3)) {
                ImGui::TableSetupColumn("Driver");
                ImGui::TableSetupColumn("Follower");
                ImGui::TableSetupColumn("");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                auto it = pairs.begin();
                while (it != pairs.end()) {
                    totalRatio *= it->getRatio();
                    ImGui::TableNextColumn();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%dT", it->getDriver().numTeeth);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%dT", it->getFollower().numTeeth);
                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::Button(ICON_FA_TRASH_ALT)) {
                        pairs.erase(it);
                    } else {
                        ++it;
                    }
                }
                ImGui::EndTable();
            }
            if (ImGui::Button(ICON_FA_PLUS" Add Gear Pair")) {
                pairs.emplace_back(gears::GEAR_8T, gears::GEAR_8T);
            }
            ImGui::Separator();
            ImGui::Text("Gear Ratio: %ld:%ld", totalRatio.getA(), totalRatio.getB());

            if (ImGui::Button("Close")) {
                *show = false;
            }
            ImGui::End();
        }
    }
}
