//
// Created by bb1950328 on 20.01.2021.
//

#include "gui.h"
#include "../latest_log_messages_tank.h"

namespace gui {
    namespace {
        ImVec4 levelToColor(const unsigned char level) {
            switch (level) {
                case 0:
                case 1: return ImVec4(0.5, 0.5, 0.5, 1.0);
                case 2: return ImVec4(0.1, 0.0, 1.0, 1.0);
                case 3: return ImVec4(1.0, 0.5, 0.05, 1.0);
                case 4: return ImVec4(1.0, 0.0, 0.0, 1.0);
                case 5: return ImVec4(1.0, 0.0, 1.0, 1.0);
                default: return ImVec4(1.0, 1.0, 1.0, 1.0);
            }
        }
        const char *levelToText(const unsigned char level) {
            switch (level) {
                case 0: return "TRACE";
                case 1: return "DEBUG";
                case 2: return "INFO";
                case 3: return "WARN";
                case 4: return "ERROR";
                case 5: return "CRITICAL";
                default: return "?";
            }
        }
    }
    
    void windows::drawLogWindow(bool *show){
        if (ImGui::Begin(WINDOW_NAME_LOG, show)) {
            static int minLevel;
            ImGui::PushStyleColor(ImGuiCol_FrameBg, levelToColor(minLevel));
            ImGui::DragInt("Level", &minLevel, 0.02f, 0, 5, levelToText(minLevel));
            ImGui::PopStyleColor();

            auto tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY;
            if (ImGui::BeginTable("Log", 3, tableFlags)) {
                ImGui::TableSetupColumn("Time");
                ImGui::TableSetupColumn("Level");
                ImGui::TableSetupColumn("Message");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();
                
                auto it = latest_log_messages_tank::getIterator();
                while (it.getCurrent() != nullptr) {
                    const auto message = *it.getCurrent();
                    it.operator++();
                    if (message.level < minLevel) {
                        continue;
                    }
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", *message.formattedTime);
                    
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextColored(levelToColor(message.level), "%s", levelToText(message.level));

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", *message.message);

                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}