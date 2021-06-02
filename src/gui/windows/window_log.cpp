#include <tinyfiledialogs.h>
#include <filesystem>
#include <fstream>
#include "../gui.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "../../latest_log_messages_tank.h"

#include "window_log.h"

namespace gui::windows::log {
    namespace {
        constexpr int NUM_LOG_FILTER_PATTERNS = 2;
        char const *logFilterPatterns[NUM_LOG_FILTER_PATTERNS] = {"*.log", "*.txt"};

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

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            static int minLevel;
            static float fontSize = ImGui::GetFontSize();
            ImGui::PushStyleColor(ImGuiCol_FrameBg, levelToColor(minLevel));
            ImGui::SetNextItemWidth(fontSize * 5);
            ImGui::DragInt("Level", &minLevel, 0.02f, 0, 5, levelToText(minLevel));
            ImGui::PopStyleColor();

            ImGui::SameLine();
            bool copyClicked = ImGui::Button(ICON_FA_CLIPBOARD" Copy");
            ImGui::SameLine();
            bool saveClicked = ImGui::Button(ICON_FA_SAVE" Save");
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_BAN" Clear")) {
                latest_log_messages_tank::clear();
            }

            std::string exportResult;
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
                    ImGui::Text("%s", message.formattedTime.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextColored(levelToColor(message.level), "%s", levelToText(message.level));

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", message.message.c_str());

                    if (copyClicked || saveClicked) {
                        exportResult += (message.formattedTime
                                         + "\t"
                                         + levelToText(message.level)
                                         + "\t"
                                         + message.message
                                         + "\n");
                    }
                }

                ImGui::EndTable();

                if (copyClicked) {
                    glfwSetClipboardString(getWindow(), exportResult.c_str());
                    spdlog::debug("log copied to clipboard");
                }
                if (saveClicked) {
                    const auto path = std::filesystem::path(tinyfd_saveFileDialog("Save log to file", nullptr,
                                                            NUM_LOG_FILTER_PATTERNS, logFilterPatterns, nullptr));
                    std::ofstream outFile(path);
                    outFile << exportResult;
                    outFile.close();
                    spdlog::debug("log saved to {}", path.string());
                }
            }
        }
        ImGui::End();
    }
}