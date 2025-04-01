#include "window_ldraw_library_updater.h"
#include "../../../controller.h"
#include "../../../utilities/ldraw_library_updater.h"
#include "../../gui.h"

namespace bricksim::gui::windows::utilities::ldraw_library_updater {
    using namespace bricksim::ldraw_library_updater;

    void draw(Data& data) {
        const auto lastVisible = data.visible;
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            auto& state = getState();

            switch (state.step) {
                case Step::INACTIVE:
                    controller::addBackgroundTask("Initialize LDraw Library Updater", []() { getState().initialize(); });
                case Step::INITIALIZING:
                {
                    const auto progressMessage = fmt::format("Initializing {}%", state.initializingProgress * 100.f);
                    ImGui::ProgressBar(state.initializingProgress, ImVec2(-FLT_MIN, 0), progressMessage.c_str());
                    break;
                }
                case Step::CHOOSE_ACTION:
                case Step::UPDATE_INCREMENTAL:
                case Step::UPDATE_COMPLETE:
                    ImGui::SeparatorText("Current Status");
                    ImGui::Text("Library base path: %s", ldr::file_repo::get().getBasePath().string().c_str());
                    ImGui::Text("Current LDConfig date: %d-%02d-%02d", (int)state.currentReleaseDate.year(), (unsigned int)state.currentReleaseDate.month(), (unsigned int)state.currentReleaseDate.day());
                    ImGui::Text("Current release ID: %s", state.currentReleaseId.c_str());

                    ImGui::SeparatorText("Incremental Update");
                    if (state.incrementalUpdates.empty()) {
                        ImGui::PushStyleColor(ImGuiCol_Text, color::GREEN);
                        ImGui::Text(ICON_FA_CHECK " no incremental updates found. This usually means that your library is up to date.");
                        ImGui::PopStyleColor();
                    } else {
                        if (ImGui::BeginTable("incrementalUpdates", state.step == Step::UPDATE_INCREMENTAL ? 4 : 3)) {
                            ImGui::TableSetupColumn("ID");
                            ImGui::TableSetupColumn("Date");
                            ImGui::TableSetupColumn("Size");
                            if (state.step == Step::UPDATE_INCREMENTAL) {
                                ImGui::TableSetupColumn("Progress");
                            }
                            ImGui::TableHeadersRow();

                            for (int i = 0; i < state.incrementalUpdates.size(); ++i) {
                                const auto& dist = state.incrementalUpdates[i];

                                ImGui::TableNextColumn();
                                ImGui::Text("%s", dist.id.c_str());

                                ImGui::TableNextColumn();
                                ImGui::Text("%d-%02d-%02d", (int)dist.date.year(), (unsigned int)dist.date.month(), (unsigned int)dist.date.day());

                                ImGui::TableNextColumn();
                                ImGui::Text("%s", stringutil::formatBytesValue(dist.size).c_str());

                                if (state.step == Step::UPDATE_INCREMENTAL) {
                                    ImGui::TableNextColumn();
                                    ImGui::ProgressBar(i < state.incrementalUpdateProgress.size() ? state.incrementalUpdateProgress[i] : 0.f);
                                }
                            }
                            ImGui::EndTable();
                        }
                        ImGui::Text("Total download size for incremental update: %s", stringutil::formatBytesValue(state.getIncrementalUpdateTotalSize()).c_str());
                        ImGui::BeginDisabled(state.step != Step::CHOOSE_ACTION);
                        if (ImGui::Button("Do Incremental Update")) {
                            controller::addBackgroundTask("Update LDraw Library", []() { getState().doIncrementalUpdate(); });
                        }
                        ImGui::EndDisabled();
                    }

                    ImGui::SeparatorText("Complete Update");
                    if (state.completeDistribution) {
                        ImGui::Text("Complete update download size: %s", stringutil::formatBytesValue(state.completeDistribution->size).c_str());
                        ImGui::BeginDisabled(state.step != Step::CHOOSE_ACTION);
                        if (ImGui::Button("Do Complete Update")) {
                            controller::addBackgroundTask("Update LDraw Library", []() { getState().doCompleteUpdate(); });
                        }
                        ImGui::EndDisabled();
                        if (state.step == Step::UPDATE_COMPLETE && state.completeUpdateProgress.has_value()) {
                            ImGui::ProgressBar(*state.completeUpdateProgress);
                        }
                    } else {
                        ImGui::Text("Complete distribution is not available");
                    }

                    break;
                case Step::FINISHED:
                    ImGui::Text("Update finished.");
                    ImGui::Text("Your parts library is up to date");
                    break;
            }
        }
        if (lastVisible && !data.visible) {
            resetState();
        }
        ImGui::End();
    }
}