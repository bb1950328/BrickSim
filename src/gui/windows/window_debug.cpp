#include "../../controller.h"
#include "../../graphics/scene.h"
#include "../../helpers/util.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "../../metrics.h"
#include "../../types.h"
#include "../gui.h"
#include <glm/gtx/io.hpp>
#include <map>
#include <memory>
#include <sstream>

#include "window_debug.h"
#include "window_mesh_inspector.h"

namespace bricksim::gui::windows::debug {
    namespace {
        void drawGeneralTab();

        void drawPerformanceTab();

        void drawCameraTab();

        constexpr size_t MESHES_TAB_BUF_SIZE = 32;

        void printSceneDescription(uomap_t<scene_id_t, std::shared_ptr<graphics::Scene>>& allScenes, scene_id_t sceneId, char* buf) {
            snprintf(buf, MESHES_TAB_BUF_SIZE, "%d (%d*%d)", sceneId, allScenes[sceneId]->getImageSize().x, allScenes[sceneId]->getImageSize().y);
        }

        void drawColorLabel(const ldr::ColorReference& colorRef);

        void drawMeshesList(char* buf, const std::shared_ptr<graphics::Scene>& selectedScene, float meshListTableHeight);

        void drawMeshesTab();

        void drawMeshesList(char* buf, const std::shared_ptr<graphics::Scene>& selectedScene, float meshListTableHeight) {
            const auto& meshes = selectedScene->getMeshCollection().getUsedMeshes();
            constexpr size_t maxSearchQueryLength = 32;
            static char searchQuery[maxSearchQueryLength] = {0};
            static std::shared_ptr<mesh::Mesh> currentlyInspectingMesh = nullptr;
            ImGui::InputText(ICON_FA_FILTER " Mesh Name Filter", searchQuery, maxSearchQueryLength);
            if (ImGui::BeginChild("##meshesListWrapper", ImVec2(0.0f, meshListTableHeight))) {
                if (ImGui::BeginTable("Meshes", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti)) {
                    enum Column {
                        NAME,
                        INSTANCE_COUNT,
                        TRIANGLE_COUNT,
                        BUTTON,
                    };
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.0f, NAME);
                    ImGui::TableSetupColumn("# Inst.", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 3.5f, INSTANCE_COUNT);
                    ImGui::TableSetupColumn("# Tri.", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 2.75f, TRIANGLE_COUNT);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, ImGui::GetFontSize() * 5.0f, BUTTON);
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();

                    std::vector<std::shared_ptr<mesh::Mesh>> sortedMeshes;
                    bool noSearchQuery = strlen(searchQuery) == 0;
                    if (noSearchQuery) {
                        sortedMeshes.reserve(meshes.size());
                    }
                    for (const auto& item: meshes) {
                        if (noSearchQuery || util::containsIgnoreCase(item->name, searchQuery)) {
                            sortedMeshes.push_back(item);
                        }
                    }
                    ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
                    std::sort(sortedMeshes.begin(), sortedMeshes.end(), [&](const std::shared_ptr<mesh::Mesh>& a, const std::shared_ptr<mesh::Mesh>& b) {
                        for (int i = 0; i < sortSpecs->SpecsCount; ++i) {
                            const auto& spec = sortSpecs->Specs[i];
#ifdef BRICKSIM_PLATFORM_MACOS
                            //for some reason, the GitHub Actions MacOS Compiler can't compile the spaceship operator code
                            //so here is a alternative.
                            //todo fix that so this alternative can be removed
                            if (spec.ColumnUserID == NAME) {
                                const auto cmp = a->name.compare(b->name);
                                if (cmp > 0) {
                                    return false;
                                } else if (cmp < 0) {
                                    return true;
                                }
                            } else if (spec.ColumnUserID == INSTANCE_COUNT) {
                                if (a->instances.size() > b->instances.size()) {
                                    return false;
                                } else if (a->instances.size() < b->instances.size()) {
                                    return true;
                                }
                            } else if (spec.ColumnUserID == TRIANGLE_COUNT) {
                                if (a->getTriangleCount() > b->getTriangleCount()) {
                                    return false;
                                } else if (a->getTriangleCount() < b->getTriangleCount()) {
                                    return true;
                                }
                            }
#else

                            std::strong_ordering cmp = std::strong_ordering::equal;
                            switch (spec.ColumnUserID) {
                                case NAME: cmp = a->name <=> b->name;
                                    break;
                                case INSTANCE_COUNT:cmp = a->instances.size() <=> b->instances.size();
                                    break;
                                case TRIANGLE_COUNT:cmp = a->getTriangleCount() <=> b->getTriangleCount();
                                    break;
                                default:break;
                            }
                            if (cmp == std::strong_ordering::less) {
                                return spec.SortDirection == ImGuiSortDirection_Ascending;
                            } else if (cmp == std::strong_ordering::greater) {
                                return spec.SortDirection == ImGuiSortDirection_Descending;
                            }
#endif
                        }
                        return false;
                    });

                    for (const auto& mesh: sortedMeshes) {
                        ImGui::TableNextRow();

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", mesh->name.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text("%zu", mesh->instances.size());

                        ImGui::TableNextColumn();
                        ImGui::Text("%zu", mesh->getTriangleCount());

                        ImGui::TableNextColumn();
                        if (mesh == currentlyInspectingMesh) {
                            ImGui::Text(ICON_FA_CHECK " Inspecting");
                        } else {
                            snprintf(buf, MESHES_TAB_BUF_SIZE, ICON_FA_INFO " Inspect##%zu", robin_hood::hash<std::string>()(mesh->name));
                            if (ImGui::Button(buf)) {
                                mesh_inspector::setCurrentlyInspectingMesh(mesh);
                                *isVisible(Id::MESH_INSPECTOR) = true;
                                currentlyInspectingMesh = mesh;
                            }
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::EndChild();
            }
        }

        void drawGeneralTab() {
            if (ImGui::BeginTabItem("General")) {
                const auto& bgTasks = controller::getBackgroundTasks();
                if (!bgTasks.empty()) {
                    ImGui::Text("%zu background tasks:", bgTasks.size());
                    for (const auto& task: bgTasks) {
                        ImGui::BulletText("%s", task.second.getName().c_str());
                    }
                }

                if (ImGui::Button(ICON_FA_SYNC " Reread element tree now")) {
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
                ImGui::Text(ICON_FA_CHART_LINE " Application render average %.3f ms/frame (%.1f FPS)", arrPtr[endIdx], 1000.0 / arrPtr[endIdx]);
                ImGui::PlotLines("ms/frame", arrPtr, count, startIdx);
                ImGui::Text(ICON_FA_STOPWATCH " Last 3D View render time: %.3f ms", metrics::lastSceneRenderTimeMs);
                ImGui::Text(ICON_FA_MEMORY " Total graphics buffer size: %s", util::formatBytesValue(metrics::vramUsageBytes).c_str());
                ImGui::Text(ICON_FA_IMAGES " Total thumbnail buffer size: %s", util::formatBytesValue(metrics::thumbnailBufferUsageBytes).c_str());
                ImGui::Text("Memory saved by deleting vertex data from RAM: %s", util::formatBytesValue(metrics::memorySavedByDeletingVertexData).c_str());
                ImGui::Text(ICON_FA_SYNC " Last element tree reread: %.2f ms", metrics::lastElementTreeRereadMs);
                ImGui::Text(ICON_FA_HISTORY " Last thumbnail render time: %.2f ms", metrics::lastThumbnailRenderingTimeMs);

                constexpr auto performanceTableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable;

                if (ImGui::BeginTable("Mainloop Time Points", 2, performanceTableFlags)) {
                    for (const auto& timePointsUs: metrics::mainloopTimePointsUs) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", timePointsUs.first);
                        ImGui::TableNextColumn();
                        ImGui::Text("%u µs", timePointsUs.second);
                    }
                    ImGui::EndTable();
                }

                if (ImGui::BeginTable(ICON_FA_WINDOW_RESTORE " Window drawing times", 2, performanceTableFlags)) {
                    for (const auto& item: metrics::lastWindowDrawingTimesUs) {
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

        void drawCameraTab() {
            if (ImGui::BeginTabItem(ICON_FA_CAMERA " Camera")) {
                auto camera = controller::getMainSceneCamera();
                if (ImGui::BeginTable("##cameraTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                    ImGui::TableNextRow();
                    auto cameraPos = camera->getCameraPos();
                    ImGui::TableNextColumn();
                    ImGui::Text("CameraPos");
                    ImGui::TableNextColumn();
                    ImGui::Text("[%f, %f, %f]", cameraPos.x, cameraPos.y, cameraPos.z);

                    ImGui::TableNextRow();
                    auto targetPos = camera->getTargetPos();
                    ImGui::TableNextColumn();
                    ImGui::Text("TargetPos");
                    ImGui::TableNextColumn();
                    ImGui::Text("[%f, %f, %f]", targetPos.x, targetPos.y, targetPos.z);

                    ImGui::TableNextRow();
                    auto viewMatrix = camera->getViewMatrix();
                    ImGui::TableNextColumn();
                    ImGui::Text("ViewMatrix");
                    ImGui::TableNextColumn();
                    std::stringstream sstream;
                    sstream << viewMatrix;
                    const auto viewMatrixStr = sstream.str();
                    ImGui::Text("%s", viewMatrixStr.c_str());

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Distance");
                    ImGui::TableNextColumn();
                    ImGui::Text("%f", camera->getDistance());

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Pitch, Yaw");
                    ImGui::TableNextColumn();
                    ImGui::Text("%f, %f", camera->getPitch(), camera->getYaw());

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
        }

        void drawMeshesTab() {
            if (ImGui::BeginTabItem("Meshes")) {
                auto allScenes = graphics::scenes::getAll();
                static scene_id_t selectedSceneId = graphics::scenes::MAIN_SCENE_ID;
                static char buf[MESHES_TAB_BUF_SIZE];
                printSceneDescription(allScenes, selectedSceneId, buf);
                if (ImGui::BeginCombo("Scene", buf)) {
                    for (const auto& scene: allScenes) {
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

                auto& selectedScene = allScenes[selectedSceneId];
                float meshListTableHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFontSize() * 3;
                drawMeshesList(buf, selectedScene, meshListTableHeight);
                ImGui::Text("%zu Meshes", selectedScene->getMeshCollection().getUsedMeshes().size());

                ImGui::EndTabItem();
            }
        }

    }

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            if (ImGui::BeginTabBar("##debugTabBar")) {
                drawGeneralTab();
                drawPerformanceTab();
                drawMeshesTab();
                drawCameraTab();

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}