#include "../../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../../../tools/gears.h"
#include "../../gui.h"
#include <list>

#include "window_gear_ratio_calculator.h"

namespace bricksim::gui::windows::tools::gear_ratio_calculator {
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            static std::list<gears::GearPair> pairs;
            if (pairs.empty()) {
                pairs.emplace_back(gears::GEAR_8T, gears::GEAR_8T);
            }
            auto totalRatio = Fraction(1, 1);
            auto maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 4;
            static char labelBuf[14];

            if (ImGui::BeginTable("##gearPairs", 3, ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Driver");
                ImGui::TableSetupColumn("Follower");
                ImGui::TableSetupColumn("");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                auto it = pairs.begin();
                int iComboBox = 0;
                while (it != pairs.end()) {
                    totalRatio *= it->getRatio();
                    ImGui::TableNextRow();
                    if (it == pairs.begin()) {
                        const auto lastColWidth = ImGui::GetFontSize() * 1.8f;
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushItemWidth((maxWidth - lastColWidth) / 2);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushItemWidth((maxWidth - lastColWidth) / 2);
                        ImGui::TableSetColumnIndex(2);
                        ImGui::PushItemWidth(lastColWidth);
                    }
                    ImGui::TableSetColumnIndex(0);
                    sprintf(labelBuf, "##%d", iComboBox);
                    ++iComboBox;
                    if (ImGui::BeginCombo(labelBuf, it->getDriver()->description)) {
                        for (const auto& gear: gears::ALL_GEARS) {
                            const auto id1 = it->getDriver()->id;
                            const auto id2 = gear->id;
                            const bool is_selected = (id1 == id2);
                            if (ImGui::Selectable(gear->description, is_selected)) {
                                const gears::gear_t follower = it->getFollower();
                                it = pairs.erase(it);
                                it = pairs.emplace(it, gear, follower);
                            }

                            if (is_selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    sprintf(labelBuf, "##%d", iComboBox);
                    ++iComboBox;
                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::BeginCombo(labelBuf, it->getFollower()->description)) {
                        for (const auto& gear: gears::ALL_GEARS) {
                            const bool is_selected = (it->getFollower()->id == gear->id);
                            if (ImGui::Selectable(gear->description, is_selected)) {
                                const gears::gear_t driver = it->getDriver();
                                it = pairs.erase(it);
                                it = pairs.emplace(it, driver, gear);
                            }

                            if (is_selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::Button(ICON_FA_TRASH_CAN)) {
                        it = pairs.erase(it);
                    } else {
                        ++it;
                    }
                }
                ImGui::EndTable();
            }
            if (ImGui::Button(ICON_FA_PLUS " Add Gear Pair")) {
                pairs.emplace_back(gears::GEAR_8T, gears::GEAR_8T);
            }
            ImGui::Separator();
            ImGui::Text("Gear Ratio: %ld:%ld", totalRatio.getA(), totalRatio.getB());

            ImGui::End();
        }
    }
}
