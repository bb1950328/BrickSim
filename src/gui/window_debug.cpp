

#include "gui.h"
#include "../controller.h"
#include "../metrics.h"

namespace gui {
    namespace {
        void drawGeneralTab() {
            if (ImGui::BeginTabItem("General")) {
                const auto &bgTasks = controller::getBackgroundTasks();
                if (!bgTasks.empty()) {
                    ImGui::Text("%lu background tasks:", bgTasks.size());
                    for (const auto &task : bgTasks) {
                        ImGui::BulletText("%s", task.second->getName().c_str());
                    }
                }

                if (ImGui::Button(ICON_FA_SYNC" Reread element tree now")) {
                    controller::setElementTreeChanged(true);
                }

                ImGui::EndTabItem();
            }
        }

        void drawPerformanceTab() {
            if (ImGui::BeginTabItem("Performance")) {
                const auto lastFrameTimes = controller::getLastFrameTimes();
                const auto count = std::get<0>(lastFrameTimes);
                const auto arrPtr = std::get<1>(lastFrameTimes);
                const auto startIdx = std::get<2>(lastFrameTimes);
                const auto endIdx = (startIdx - 1) % count;
                ImGui::Text(ICON_FA_CHART_LINE" Application render average %.3f ms/frame (%.1f FPS)", arrPtr[endIdx], 1000.0 / arrPtr[endIdx]);
                ImGui::PlotLines("ms/frame", arrPtr, count, startIdx);
                ImGui::Text(ICON_FA_STOPWATCH" Last 3D View render time: %.3f ms", metrics::lastSceneRenderTimeMs);
                ImGui::Text(ICON_FA_MEMORY" Total graphics buffer size: %s", util::formatBytesValue(metrics::vramUsageBytes).c_str());
                ImGui::Text(ICON_FA_IMAGES" Total thumbnail buffer size: %s", util::formatBytesValue(metrics::thumbnailBufferUsageBytes).c_str());
                ImGui::Text(ICON_FA_SYNC" Last element tree reread: %.2f ms", metrics::lastElementTreeRereadMs);
                ImGui::Text(ICON_FA_HISTORY" Last thumbnail render time: %.2f ms", metrics::lastThumbnailRenderingTimeMs);

                constexpr auto performanceTableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable;

                if (ImGui::BeginTable("Mainloop Time Points", 2, performanceTableFlags)) {
                    for (const auto &timePointsUs : metrics::mainloopTimePointsUs) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", timePointsUs.first);
                        ImGui::TableNextColumn();
                        ImGui::Text("%u µs", timePointsUs.second);
                    }
                    ImGui::EndTable();
                }

                if (ImGui::BeginTable(ICON_FA_WINDOW_RESTORE" Window drawing times", 2, performanceTableFlags)) {
                    for (const auto &item : metrics::lastWindowDrawingTimesUs) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", item.first.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%.1f µs", item.second);
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }
        }

        constexpr size_t MESHES_TAB_BUF_SIZE = 32;

        void printSceneDescription(std::map<scene_id_t, std::shared_ptr<Scene>> &allScenes, scene_id_t sceneId, char *buf) {
            snprintf(buf, MESHES_TAB_BUF_SIZE, "%d (%d*%d)", sceneId, allScenes[sceneId]->getImageSize().x, allScenes[sceneId]->getImageSize().y);
        }

        void drawMeshesTab() {
            if (ImGui::BeginTabItem("Meshes")) {
                auto allScenes = scenes::getAll();
                static scene_id_t selectedSceneId = scenes::MAIN_SCENE_ID;
                static char buf[MESHES_TAB_BUF_SIZE];
                printSceneDescription(allScenes, selectedSceneId, buf);
                if (ImGui::BeginCombo("Scene", buf)) {
                    for (const auto &scene : allScenes) {
                        const bool isSelected = scene.first == selectedSceneId;
                        printSceneDescription(allScenes, scene.first, buf);
                        if (ImGui::Selectable(buf, isSelected)) {
                            selectedSceneId = scene.first;
                        }

                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                auto &selectedScene = allScenes[selectedSceneId];

                static std::shared_ptr<Mesh> currentlyInspectingMesh = nullptr;

                bool openPopupNow = false;

                if (ImGui::BeginTable("Meshes", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Inst.");
                    ImGui::TableSetupColumn("Tri. count");
                    ImGui::TableSetupColumn("");
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();

                    for (const auto &mesh : selectedScene->getMeshCollection().getUsedMeshes()) {
                        ImGui::TableNextRow();

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", mesh->name.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text("%zu", mesh->instances.size());

                        ImGui::TableNextColumn();
                        size_t triIndices = 0;
                        for (const auto &item : mesh->triangleIndices) {
                            triIndices += item.second.size();
                        }
                        ImGui::Text("%zu", triIndices/3);

                        ImGui::TableNextColumn();
                        snprintf(buf, MESHES_TAB_BUF_SIZE, ICON_FA_INFO" Info##%zu", std::hash<std::string>()(mesh->name));
                        if (ImGui::Button(buf)) {
                            openPopupNow = true;
                            currentlyInspectingMesh = mesh;
                        }
                    }
                    ImGui::EndTable();
                }

                if (openPopupNow) {
                    ImGui::OpenPopup("MeshInspector");
                }

                if (ImGui::BeginPopupModal("MeshInspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    std::shared_ptr<Mesh> &mesh = currentlyInspectingMesh;
                    if (ImGui::BeginTabBar("##meshInspectorTabs")) {
                        if (ImGui::BeginTabItem("General")) {
                            if (ImGui::BeginTable("##meshInspectorGeneralInfoTable", 2)) {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("Name");
                                ImGui::TableNextColumn();
                                ImGui::Text("%s", mesh->name.c_str());

                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("Instances");
                                ImGui::TableNextColumn();
                                ImGui::Text("%zu", mesh->instances.size());

                                //todo add more info

                                ImGui::EndTable();
                            }
                            ImGui::EndTabItem();
                        }

                        //todo add tabs for vertices, indices and other data
                        ImGui::EndTabBar();
                    }
                    if (ImGui::Button(ICON_FA_WINDOW_CLOSE" Close")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::EndTabItem();
            }
        }
    }

    void windows::drawDebugWindow(bool *show) {
        ImGui::Begin(WINDOW_NAME_DEBUG, show);
        if (ImGui::BeginTabBar("##debugTabBar")) {
            drawGeneralTab();
            drawPerformanceTab();
            drawMeshesTab();

            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}