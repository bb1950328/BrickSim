#include "window_ldraw_library_updater.h"
#include "../../gui.h"
#include "../../../utilities/ldraw_library_updater.h"
#include "../../../controller.h"

namespace bricksim::gui::windows::utilities::ldraw_library_updater {
    using namespace bricksim::ldraw_library_updater;

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            auto& state = getState();

            switch (state.step) {
                case Step::INACTIVE:
                    controller::addBackgroundTask("Initialize LDraw Library Updater", [](){getState().initialize();});
                case Step::INITIALIZING: {
                    const auto progressMessage = fmt::format("Initializing {}%", state.initializingProgress * 100.f);
                    ImGui::ProgressBar(state.initializingProgress, ImVec2(-FLT_MIN, 0), progressMessage.c_str());
                    break;
                }
                case Step::CHOOSE_ACTION:
                    ImGui::SeparatorText("Current Status");
                    ImGui::Text("Library base path: %s", ldr::file_repo::get().getBasePath().string().c_str());
                    ImGui::Text("Current release date: %d-%02d-%02d", (int)state.currentReleaseDate.year(), (unsigned int)state.currentReleaseDate.month(), (unsigned int)state.currentReleaseDate.day());

                    ImGui::SeparatorText("Incremental Update");
                    if (ImGui::BeginTable("incrementalUpdates", 3)) {
                        ImGui::TableSetupColumn("ID");
                        ImGui::TableSetupColumn("Date");
                        ImGui::TableSetupColumn("Size");
                        ImGui::TableHeadersRow();

                        for (const auto& dist: state.incrementalUpdates) {
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", dist.id.c_str());

                            ImGui::TableNextColumn();
                            ImGui::Text("%d-%02d-%02d", (int)dist.date.year(), (unsigned int)dist.date.month(), (unsigned int)dist.date.day());

                            ImGui::TableNextColumn();
                            ImGui::Text("%s", stringutil::formatBytesValue(dist.size).c_str());
                        }
                        ImGui::EndTable();
                    }
                    ImGui::Text("Total download size for incremental update: %s", stringutil::formatBytesValue(state.getIncrementalUpdateTotalSize()).c_str());
                    if (ImGui::Button("Do Incremental Update")) {
                        state.doIncrementalUpdate();
                    }

                    ImGui::SeparatorText("Complete Update");
                    if (state.completeDistribution) {
                        ImGui::Text("Complete update download size: %s", stringutil::formatBytesValue(state.completeDistribution->size).c_str());
                        if (ImGui::Button("Do Complete Update")) {
                            state.doCompleteUpdate();
                        }
                    } else {
                        ImGui::Text("Complete distribution is not available");
                    }

                    break;
                case Step::UPDATE_INCREMENTAL:
                    break;
                case Step::UPDATE_COMPLETE:
                    break;
                case Step::FINISHED:
                    break;
            }
        }
        ImGui::End();
    }
}