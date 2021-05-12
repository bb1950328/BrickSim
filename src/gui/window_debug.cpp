#include "gui.h"
#include "../controller.h"
#include "../metrics.h"
#include <compare>

namespace gui {
    namespace {
        void drawGeneralTab();

        void drawPerformanceTab();

        constexpr size_t MESHES_TAB_BUF_SIZE = 32;

        void printSceneDescription(std::map<scene_id_t, std::shared_ptr<Scene>> &allScenes, scene_id_t sceneId, char *buf) {
            snprintf(buf, MESHES_TAB_BUF_SIZE, "%d (%d*%d)", sceneId, allScenes[sceneId]->getImageSize().x, allScenes[sceneId]->getImageSize().y);
        }

        void drawColorLabel(const LdrColorReference &colorRef);

        void drawMeshesList(char *buf, const std::shared_ptr<Scene> &selectedScene, float meshListTableHeight, std::shared_ptr<Mesh> &currentlyInspectingMesh, bool &openPopupNow);

        void drawMeshesTab();

        namespace mesh_inspector {
            void drawMeshInspectorPopup(std::shared_ptr<Mesh> &currentlyInspectingMesh, bool &openPopupNow);
            void drawGeneralTab(std::shared_ptr<Mesh> &mesh);
            void drawInstancesTab(std::shared_ptr<Mesh> &mesh);
            void drawTriangleVerticesTab(std::shared_ptr<Mesh> &mesh);
        }

        void drawMeshesList(char *buf, const std::shared_ptr<Scene> &selectedScene, float meshListTableHeight, std::shared_ptr<Mesh> &currentlyInspectingMesh, bool &openPopupNow) {
            const auto &meshes = selectedScene->getMeshCollection().getUsedMeshes();
            constexpr size_t maxSearchQueryLength = 32;
            static char searchQuery[maxSearchQueryLength] = {0};
            ImGui::InputText(ICON_FA_FILTER" Mesh Name Filter", searchQuery, maxSearchQueryLength);
            if (ImGui::BeginChild("##meshesListWrapper", ImVec2(0.0f, meshListTableHeight))) {
                if (ImGui::BeginTable("Meshes", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti)) {
                    enum Column {
                        NAME,
                        INSTANCE_COUNT,
                        TRIANGLE_COUNT,
                        BUTTON,
                    };
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.0f, NAME);
                    ImGui::TableSetupColumn("Inst.", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 2.7f, INSTANCE_COUNT);
                    ImGui::TableSetupColumn("Tri. count", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 4, TRIANGLE_COUNT);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_NoSort, ImGui::GetFontSize() * 3.4f, BUTTON);
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();


                    std::vector<std::shared_ptr<Mesh>> sortedMeshes;
                    bool noSearchQuery = strlen(searchQuery) == 0;
                    if (noSearchQuery) {
                        sortedMeshes.reserve(meshes.size());
                    }
                    for (const auto &item : meshes) {
                        if (noSearchQuery || item->name.find(searchQuery) != std::string::npos) {
                            sortedMeshes.push_back(item);
                        }
                    }
                    ImGuiTableSortSpecs *sortSpecs = ImGui::TableGetSortSpecs();
                    std::sort(sortedMeshes.begin(), sortedMeshes.end(), [&](const std::shared_ptr<Mesh> &a, const std::shared_ptr<Mesh> &b) {
                        for (int i = 0; i < sortSpecs->SpecsCount; ++i) {
                            const auto &spec = sortSpecs->Specs[i];
#ifdef BRICKSIM_PLATFORM_MACOS
                            //for some reason, the GitHub Actions MacOS Compiler can't compile the spaceship operator code
                            //so here is a alternative.
                            //todo fix that so this alternative can be removed
                            if (spec.ColumnUserID==NAME) {
                                const auto cmp = a->name.compare(b->name);
                                if (cmp > 0) {
                                    return false;
                                } else if (cmp < 0) {
                                    return true;
                                }
                            } else if (spec.ColumnUserID==INSTANCE_COUNT) {
                                if (a->instances.size() > b->instances.size()) {
                                    return false;
                                } else if (a->instances.size() < b->instances.size()) {
                                    return true;
                                }
                            } else if (spec.ColumnUserID==TRIANGLE_COUNT) {
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
                                return spec.SortDirection==ImGuiSortDirection_Ascending;
                            } else if (cmp == std::strong_ordering::greater) {
                                return spec.SortDirection==ImGuiSortDirection_Descending;
                            }
#endif
                        }
                        return false;
                    });

                    for (const auto &mesh : sortedMeshes) {
                        ImGui::TableNextRow();

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", mesh->name.c_str());

                        ImGui::TableNextColumn();
                        ImGui::Text("%zu", mesh->instances.size());

                        ImGui::TableNextColumn();
                        ImGui::Text("%zu", mesh->getTriangleCount());

                        ImGui::TableNextColumn();
                        snprintf(buf, MESHES_TAB_BUF_SIZE, ICON_FA_INFO" Info##%zu", std::hash<std::string>()(mesh->name));
                        if (ImGui::Button(buf)) {
                            openPopupNow = true;
                            currentlyInspectingMesh = mesh;
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::EndChild();
            }
        }

        void drawGeneralTab() {
            if (ImGui::BeginTabItem("General")) {
                const auto &bgTasks = controller::getBackgroundTasks();
                if (!bgTasks.empty()) {
                    ImGui::Text("%zu background tasks:", bgTasks.size());
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

        void drawColorLabel(const LdrColorReference &colorRef) {
            auto color = colorRef.get();

            const glm::vec3 &value = color->value.asGlmVector();
            const glm::vec3 &edge = color->edge.asGlmVector();
            const ImVec4 valueImVec = ImVec4(value.x, value.y, value.z, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, valueImVec);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, valueImVec);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, valueImVec);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(edge.x, edge.y, edge.z, 1.0f));

            std::string text = std::to_string(color->code) + ": " + color->name;
            ImGui::Button(text.c_str());

            ImGui::PopStyleColor(4);
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

                static bool openPopupNow = false;

                float meshListTableHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFontSize() * 3;
                drawMeshesList(buf, selectedScene, meshListTableHeight, currentlyInspectingMesh, openPopupNow);

                ImGui::Text("%zu Meshes", selectedScene->getMeshCollection().getUsedMeshes().size());

                if (openPopupNow) {
                    ImGui::OpenPopup("MeshInspector");
                }

                mesh_inspector::drawMeshInspectorPopup(currentlyInspectingMesh, openPopupNow);

                ImGui::EndTabItem();
            }
        }

        namespace mesh_inspector {
            void drawMeshInspectorPopup(std::shared_ptr<Mesh> &currentlyInspectingMesh, bool &openPopupNow) {
                if (ImGui::BeginPopupModal("MeshInspector", &openPopupNow, ImGuiWindowFlags_None)) {
                    std::shared_ptr<Mesh> &mesh = currentlyInspectingMesh;
                    if (ImGui::BeginTabBar("##meshInspectorTabs")) {
                        drawGeneralTab(mesh);
                        drawInstancesTab(mesh);
                        drawTriangleVerticesTab(mesh);

                        ImGui::EndTabBar();
                    }
                    if (ImGui::Button(ICON_FA_WINDOW_CLOSE" Close")) {
                        openPopupNow = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }

            void drawTriangleVerticesTab(std::shared_ptr<Mesh> &mesh) {
                if (ImGui::BeginTabItem("Triangle Vertices")) {
                    const char *items[] = {"Vertices & Indices separate", "\"Inline\" Indices"};
                    static int currentMode = 0;
                    ImGui::Combo("Mode", &currentMode, items, std::size(items));

                    static LdrColorReference currentColor = ldr_color_repo::NO_COLOR_CODE;
                    bool selectFirst = mesh->triangleVertices.find(currentColor) == mesh->triangleVertices.end();
                    if (selectFirst) {
                        currentColor = mesh->triangleVertices.begin()->first;
                    }
                    if (ImGui::BeginCombo("Color", currentColor.get()->name.c_str())) {
                        for (const auto &vertexGroup : mesh->triangleVertices) {
                            auto color = vertexGroup.first;
                            const bool isSelected = currentColor == color;
                            if (ImGui::Selectable(color.get()->name.c_str(), isSelected)) {
                                currentColor = color;
                            }

                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }

                    bool copyVerticesToClipboard = ImGui::Button(ICON_FA_CLIPBOARD" Copy Vertices as CSV");
                    bool copyIndicesToClipboard = false;
                    if (currentMode == 0) {
                        ImGui::SameLine();
                        copyIndicesToClipboard = ImGui::Button(ICON_FA_CLIPBOARD" Copy Indices");
                    }
                    std::stringstream toClipboard;

                    if (copyVerticesToClipboard) {
                        toClipboard << "pos.x;pos.y;pos.z;pos.w;normal.x;normal.y;normal.z" << std::endl;
                    }

                    auto drawVertexRow = [&copyVerticesToClipboard, &toClipboard](unsigned int index, const TriangleVertex &vertex) {
                        ImGui::TableNextRow();
                        ImGui::PushItemWidth(-1.0f);

                        ImGui::TableNextColumn();
                        ImGui::Text("%u", index);

                        ImGui::TableNextColumn();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                        ImGui::Text("%.2f", vertex.position.x);
                        ImGui::PopStyleColor();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
                        ImGui::SameLine();
                        ImGui::Text("%.2f", vertex.position.y);
                        ImGui::PopStyleColor();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4, 0.5, 1, 1));
                        ImGui::SameLine();
                        ImGui::Text("%.2f", vertex.position.z);
                        ImGui::PopStyleColor();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1));
                        ImGui::SameLine();
                        ImGui::Text("%.2f", vertex.position.w);
                        ImGui::PopStyleColor();


                        ImGui::TableNextColumn();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                        ImGui::Text("%.2f", vertex.normal.x);
                        ImGui::PopStyleColor();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
                        ImGui::SameLine();
                        ImGui::Text("%.2f", vertex.normal.y);
                        ImGui::PopStyleColor();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4, 0.5, 1, 1));
                        ImGui::SameLine();
                        ImGui::Text("%.2f", vertex.normal.z);
                        ImGui::PopStyleColor();

                        ImGui::PopItemWidth();

                        if (copyVerticesToClipboard) {
                            toClipboard << vertex.position.x << ';' << vertex.position.y << ';' << vertex.position.z << ';' << vertex.position.w;
                            toClipboard << ';' << vertex.normal.x << ';' << vertex.normal.y << ';' << vertex.normal.z << std::endl;
                        }
                    };

                    float totalAvailableHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFontSize() * 2;
                    float verticesTableHeight = currentMode == 0 ? totalAvailableHeight / 2 : totalAvailableHeight;
                    float indicesTableHeight = currentMode == 0 ? verticesTableHeight : 0.0f;
                    const auto &vertexList = mesh->triangleVertices[currentColor];
                    const auto &indexList = mesh->triangleIndices[currentColor];
                    if (ImGui::BeginChild("##triangleVerticesTableWrapper", ImVec2(0.0f, verticesTableHeight))) {
                        if (ImGui::BeginTable("##triangleVerticesTable", 3, ImGuiTableFlags_Borders)) {
                            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 3);
                            ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthStretch, 0.5f);
                            ImGui::TableSetupColumn("Normal", ImGuiTableColumnFlags_WidthStretch, 0.5f);
                            ImGui::TableHeadersRow();

                            if (currentMode == 0) {
                                for (int index = 0; index < vertexList.size(); ++index) {
                                    drawVertexRow(index, vertexList[index]);
                                }
                            } else {
                                for (const auto &index : indexList) {
                                    drawVertexRow(index, vertexList[index]);
                                }
                            }

                            ImGui::EndTable();
                        }
                        ImGui::EndChild();
                    }

                    if (currentMode == 0 && ImGui::BeginChild("##triangleIndicesTableWrapper", ImVec2(0.0f, indicesTableHeight))) {
                        if (ImGui::BeginTable("##triangleIndicesTable", 1, ImGuiTableFlags_Borders)) {
                            for (const auto &index : indexList) {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("%u", index);
                                if (copyIndicesToClipboard) {
                                    toClipboard << index << std::endl;
                                }
                            }
                            ImGui::EndTable();
                        }
                        ImGui::EndChild();
                    }

                    if (copyVerticesToClipboard | copyIndicesToClipboard) {
                        std::string tmp = toClipboard.str();
                        glfwSetClipboardString(getWindow(), tmp.c_str());
                    }

                    ImGui::EndTabItem();
                }
            }

            void drawInstancesTab(std::shared_ptr<Mesh> &mesh) {
                if (ImGui::BeginTabItem("Instances")) {
                    ImGui::Text("%zu Instances:", mesh->instances.size());
                    float instancesTableHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFontSize() * 2;
                    if (ImGui::BeginChild("##instancesTableChild", ImVec2(0.0f, instancesTableHeight))) {
                        constexpr auto flags = ImGuiTableFlags_Borders;
                        if (ImGui::BeginTable("##instancesTable", 6, flags)) {
                            ImGui::TableSetupColumn("Scene", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 3);
                            ImGui::TableSetupColumn("Layer", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 3);
                            ImGui::TableSetupColumn("ElementId", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 4.5f);
                            ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 5);
                            ImGui::TableSetupColumn("Selected", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 4);
                            ImGui::TableSetupColumn("Transformation", ImGuiTableColumnFlags_WidthStretch, 1.0f);
                            ImGui::TableSetupScrollFreeze(0, 1);
                            ImGui::TableHeadersRow();

                            for (const auto &inst : mesh->instances) {
                                ImGui::TableNextRow();

                                ImGui::TableNextColumn();
                                ImGui::Text("%d", inst.scene);

                                ImGui::TableNextColumn();
                                ImGui::Text("%d", inst.layer);

                                ImGui::TableNextColumn();
                                ImGui::Text("%x", inst.elementId);

                                ImGui::TableNextColumn();
                                drawColorLabel(inst.color);

                                ImGui::TableNextColumn();
                                ImGui::Text(inst.selected ? ICON_FA_CHECK_SQUARE : ICON_FA_SQUARE);

                                ImGui::TableNextColumn();
                                const auto &mat = inst.transformation;
                                ImGui::Text("%8.4f, %8.4f, %8.4f, %8.4f", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
                                ImGui::Text("%8.4f, %8.4f, %8.4f, %8.4f", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
                                ImGui::Text("%8.4f, %8.4f, %8.4f, %8.4f", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
                                ImGui::Text("%8.4f, %8.4f, %8.4f, %8.4f", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
                            }
                            ImGui::EndTable();
                        }
                        ImGui::EndChild();
                    }
                    ImGui::EndTabItem();
                }
            }

            void drawGeneralTab(std::shared_ptr<Mesh> &mesh) {
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

                    if (ImGui::BeginTable("##meshInspectorBufferIdsTable", 5)) {
                        ImGui::TableSetupColumn("Color");
                        ImGui::TableSetupColumn("VAO");
                        ImGui::TableSetupColumn("Vertex VBO");
                        ImGui::TableSetupColumn("Instance VBO");
                        ImGui::TableSetupColumn("EBO");
                        ImGui::TableHeadersRow();

                        for (const auto &vao : mesh->VAOs) {
                            auto colorRef = vao.first;
                            ImGui::TableNextRow();

                            ImGui::TableNextColumn();
                            drawColorLabel(colorRef);

                            ImGui::TableNextColumn();
                            ImGui::Text("%u", vao.second);

                            ImGui::TableNextColumn();
                            ImGui::Text("%u", mesh->vertexVBOs[colorRef]);

                            ImGui::TableNextColumn();
                            ImGui::Text("%u", mesh->instanceVBOs[colorRef]);

                            ImGui::TableNextColumn();
                            ImGui::Text("%u", mesh->EBOs[colorRef]);
                        }
                        ImGui::EndTable();
                    }

                    if (ImGui::CollapsingHeader("Instance Scene/Layer Ranges")) {
                        for (const auto &sceneMap : mesh->instanceSceneLayerRanges) {
                            const auto text = "Scene " + std::to_string((int) sceneMap.first);
                            if (ImGui::TreeNode(text.c_str())) {
                                for (const auto &range : sceneMap.second) {
                                    ImGui::BulletText("Layer %d: start=%u, end=%u, count=%u", (int) range.first, range.second.start, range.second.start + range.second.count, range.second.count);
                                }
                                ImGui::TreePop();
                            }
                        }
                    }

                    ImGui::EndTabItem();
                }
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