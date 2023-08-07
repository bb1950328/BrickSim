#include "../../connection/engine.h"
#include "../../controller.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../../metrics.h"
#include "../gui.h"
#include <cinttypes>
#include <glm/gtx/io.hpp>
#include <glm/gtx/string_cast.hpp>
#include <map>
#include <memory>

#include "../../connection/connection_check.h"
#include "../../connection/visualization/connection_graphviz_generator.h"
#include "../../helpers/graphviz_wrapper.h"
#include "imgui_internal.h"
#include "spdlog/spdlog.h"
#include "window_debug.h"
#include "window_mesh_inspector.h"
#include <fstream>
#include <future>
#include <sstream>
#include <tinyfiledialogs.h>

namespace bricksim::gui::windows::debug {
    namespace {
        void drawGeneralTab();

        void drawPerformanceTab();

        void drawCameraTab();

        void drawMeshesTab();

        void drawConnectionTab();

        constexpr size_t MESHES_TAB_BUF_SIZE = 32;

        void printSceneDescription(scene_id_t sceneId, const std::shared_ptr<graphics::Scene>& scene, char* buf) {
            snprintf(buf, MESHES_TAB_BUF_SIZE, "%d (%d*%d)", sceneId, scene->getImageSize().x, scene->getImageSize().y);
        }

        void drawSceneSelectionCombo(scene_id_t& selectedSceneId, uomap_t<scene_id_t, std::shared_ptr<graphics::Scene>>& allScenes) {
            static char buf[MESHES_TAB_BUF_SIZE];
            printSceneDescription(selectedSceneId, allScenes[selectedSceneId], buf);
            if (ImGui::BeginCombo("Scene", buf)) {
                for (const auto& scene: allScenes) {
                    const bool isSelected = scene.first == selectedSceneId;
                    printSceneDescription(scene.first, scene.second, buf);
                    if (ImGui::Selectable(buf, isSelected)) {
                        selectedSceneId = scene.first;
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        void drawSceneSelectionCombo(scene_id_t& selectedSceneId) {
            drawSceneSelectionCombo(selectedSceneId, graphics::scenes::getAll());
        }

        void drawColorLabel(const ldr::ColorReference& colorRef);

        void drawMeshesList(char* buf, const std::shared_ptr<graphics::Scene>& selectedScene, float meshListTableHeight);

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
                        if (noSearchQuery || stringutil::containsIgnoreCase(item->name, searchQuery)) {
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
                            snprintf(buf, MESHES_TAB_BUF_SIZE, ICON_FA_INFO " Inspect##%zu", hash<std::string>()(mesh->name));
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

                for (auto& editor: controller::getEditors()) {
                    const auto& rootNode = editor->getRootNode();
                    ImGui::Text("Element tree root node version: %" PRIu64, rootNode->getVersion());
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FA_SQUARE_PLUS " Increment")) {
                        rootNode->incrementVersion();
                    }
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
                ImGui::Text(ICON_FA_STOPWATCH " Last 3D View render time: %.3f ms (%.1f FPS)", metrics::lastSceneRenderTimeMs, 1000.0 / metrics::lastSceneRenderTimeMs);
                ImGui::Text(ICON_FA_MEMORY " Total graphics buffer size: %s", stringutil::formatBytesValue(metrics::vramUsageBytes).c_str());
                ImGui::Text(ICON_FA_IMAGES " Total thumbnail buffer size: %s", stringutil::formatBytesValue(metrics::thumbnailBufferUsageBytes).c_str());
                ImGui::Text("Memory saved by deleting vertex data from RAM: %s", stringutil::formatBytesValue(metrics::memorySavedByDeletingVertexData).c_str());
                ImGui::Text(ICON_FA_ARROWS_ROTATE " Last element tree reread: %.2f ms", metrics::lastElementTreeRereadMs);
                ImGui::Text(ICON_FA_IMAGES " Last thumbnail render time: %.2f ms", metrics::lastThumbnailRenderingTimeMs);
#ifndef NDEBUG
                ImGui::Text("ldr::FileElement instance count: %zu", metrics::ldrFileElementInstanceCount);
#endif

                constexpr auto performanceTableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable;

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
                auto& allScenes = graphics::scenes::getAll();
                static scene_id_t selectedSceneId = allScenes.begin()->first;
                drawSceneSelectionCombo(selectedSceneId, allScenes);

                auto& camera = allScenes[selectedSceneId]->getCamera();
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

                    auto cadCamera = std::dynamic_pointer_cast<graphics::CadCamera>(camera);
                    if (cadCamera != nullptr) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Distance");
                        ImGui::TableNextColumn();
                        ImGui::Text("%f", cadCamera->getDistance());

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Pitch, Yaw");
                        ImGui::TableNextColumn();
                        ImGui::Text("%f, %f", cadCamera->getPitch(), cadCamera->getYaw());
                    }

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
        }

        void drawMeshesTab() {
            if (ImGui::BeginTabItem("Meshes")) {
                auto& allScenes = graphics::scenes::getAll();
                static scene_id_t selectedSceneId = allScenes.cbegin()->first;
                char buf[32];
                drawSceneSelectionCombo(selectedSceneId, allScenes);

                auto& selectedScene = allScenes[selectedSceneId];
                float meshListTableHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFontSize() * 3;
                drawMeshesList(buf, selectedScene, meshListTableHeight);
                ImGui::Text("%zu Meshes", selectedScene->getMeshCollection().getUsedMeshes().size());

                ImGui::EndTabItem();
            }
        }

        void drawConnections(const std::vector<std::shared_ptr<connection::Connection>>& connsToOtherNode) {
            uint64_t i = 1;
            for (const auto& item: connsToOtherNode) {
                if (ImGui::TreeNode(fmt::format("Connection {}", i).c_str())) {
                    ImGui::BulletText("A: %s", item->connectorA->infoStr().c_str());
                    ImGui::BulletText("B: %s", item->connectorB->infoStr().c_str());
                    for (const auto& ra: item->degreesOfFreedom.rotationPossibilities) {
                        ImGui::BulletText("Rotation possibility: origin=%s, axis=%s",
                                          stringutil::formatGLM(ra.origin).c_str(),
                                          stringutil::formatGLM(ra.axis).c_str());
                    }
                    for (const auto& sd: item->degreesOfFreedom.slideDirections) {
                        ImGui::BulletText("Slide direction: %s", stringutil::formatGLM(sd).c_str());
                    }
                    ImGui::BulletText("Completely used: %s%s",
                                      item->completelyUsedConnector[0] ? "A" : "",
                                      item->completelyUsedConnector[1] ? "B" : "");
                    ImGui::TreePop();
                }
                ++i;
            }
        }
        void drawConnectionTab() {
            if (ImGui::BeginTabItem("Connections")) {
                const auto activeEditor = controller::getActiveEditor();
                if (activeEditor == nullptr) {
                    ImGui::Text("Make an editor active and select a part to see its connections");
                } else {
                    auto& engine = activeEditor->getConnectionEngine();

                    static std::optional<std::future<bool>> updateThread;
                    static float progress;
                    if (updateThread.has_value()) {
                        ImGui::BeginDisabled();
                        ImGui::Button(ICON_FA_ROTATE " Update Connection Engine");
                        ImGui::EndDisabled();
                        ImGui::SameLine();
                        ImGui::ProgressBar(progress);
                        const auto status = updateThread->wait_for(std::chrono::milliseconds(0));
                        if (status == std::future_status::ready) {
                            updateThread = std::nullopt;
                        }
                    } else {
                        if (ImGui::Button(ICON_FA_ROTATE " Update Connection Engine")) {
                            updateThread = std::async(std::launch::async, [&engine] {
                                util::setThreadName("Debug Window / Connection Engine Update");
                                engine.update(&progress);
                                return true;
                            });
                        }
                    }

                    const auto selectedNodes = activeEditor->getSelectedNodes();
                    std::vector<std::shared_ptr<etree::MeshNode>> ldrNodes;
                    for (const auto& node: selectedNodes) {
                        const auto ldrNode = std::dynamic_pointer_cast<etree::MeshNode>(node.first);
                        if (ldrNode != nullptr) {
                            ldrNodes.push_back(ldrNode);
                        }
                    }
                    if (ldrNodes.size() == 1) {
                        const auto node = ldrNodes[0];
                        const auto& intersections = engine.getIntersections().getConnected(ldrNodes[0]);
                        ImGui::Text("Selected node intersects %zu other nodes", intersections.size());
                        for (const auto& item: intersections) {
                            ImGui::BulletText("%p %s", item.get(), item->displayName.c_str());
                        }
                        const auto& connectionsToNode = engine.getConnections().getConnections(node);
                        size_t totalConnections = 0;
                        for (const auto& [_, conns]: connectionsToNode) {
                            totalConnections += conns.size();
                        }
                        ImGui::Text("Selected node is connected to %zu other parts via %zu connections", connectionsToNode.size(), totalConnections);
                        for (const auto& [otherNode, connsToOtherNode]: connectionsToNode) {
                            std::string id = fmt::format("{} {}", otherNode->displayName, stringutil::formatGLM(otherNode->getAbsoluteTransformation()));
                            if (ImGui::TreeNode(id.c_str(), "%p %s", otherNode.get(), otherNode->displayName.c_str())) {
                                drawConnections(connsToOtherNode);
                                ImGui::TreePop();
                            }
                        }
                    } else if (ldrNodes.size() == 2) {
                        const auto intersects = engine.getIntersections().hasEdge(ldrNodes[0], ldrNodes[1]);
                        const auto& connections = engine.getConnections().getConnections(ldrNodes[0], ldrNodes[1]);
                        if (connections.empty()) {
                            if (intersects) {
                                ImGui::Text("The two selected nodes intersect, but have no connections.");
                                if (ImGui::Button("Recheck...")) {
                                    connection::VectorPairCheckResultConsumer resCon;
                                    connection::ConnectionCheck check(resCon);
                                    check.checkForConnected(ldrNodes[0], ldrNodes[1]);
                                }
                            } else {
                                ImGui::Text("The two selected nodes do not intersect.");
                            }
                        } else {
                            ImGui::Text("%lu connections between the two selected parts", connections.size());
                            drawConnections(connections);
                        }
                    } else {
                        if (ImGui::TreeNodeEx("Intersections")) {
                            if (ImGui::BeginTable("General Stats:##Intersections", 2)) {
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableNextColumn();
                                    ImGui::Text("Node count:");

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%zu", engine.getIntersections().getNodeCount());
                                }
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableNextColumn();
                                    ImGui::Text("Total intersection count:");

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%zu", engine.getIntersections().getEdgeCount());
                                }
                                ImGui::EndTable();
                            }
                            ImGui::TreePop();
                        }
                        if (ImGui::TreeNodeEx("Connections")) {
                            if (ImGui::BeginTable("General Stats:##Connections", 2)) {
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableNextColumn();
                                    ImGui::Text("Node count:");

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%zu", engine.getConnections().getAdjacencyLists().size());
                                }
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableNextColumn();
                                    ImGui::Text("Total connection count:");

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%zu", engine.getConnections().countTotalConnections());
                                }
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableNextColumn();
                                    ImGui::Text("Clique count:");

                                    ImGui::TableNextColumn();
                                    static std::size_t cliqueCount = 0;
                                    ImGui::Text("%zu", cliqueCount);
                                    ImGui::SameLine();
                                    if (ImGui::Button(ICON_FA_ROTATE "##1")) {
                                        cliqueCount = engine.getConnections().findAllCliques().size();
                                    }
                                }
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableNextColumn();
                                    ImGui::Text("Largest Clique size:");

                                    ImGui::TableNextColumn();
                                    static std::size_t largestCliqueSize = 0;
                                    ImGui::Text("%zu", largestCliqueSize);
                                    ImGui::SameLine();
                                    if (ImGui::Button(ICON_FA_ROTATE "##2")) {
                                        largestCliqueSize = 0;
                                        for (const auto& item: engine.getConnections().findAllCliques()) {
                                            largestCliqueSize = std::max(largestCliqueSize, item.size());
                                        }
                                    }
                                }
                                ImGui::EndTable();
                            }
                            ImGui::TreePop();
                        }
                        ImGui::Spacing();
                        ImGui::Text("select one or two parts to see its intersections/connections");
                    }
                    if (graphviz_wrapper::isAvailable()) {
                        if (ImGui::Button(ICON_FA_DIAGRAM_PROJECT " Render all Connections with GraphViz")) {
                            char const* outputPathChars = tinyfd_saveFileDialog(
                                    "Export Connection Graph",
                                    "connections.png",
                                    graphviz_wrapper::OUTPUT_FILE_FILTER_PATTERNS.size(),
                                    graphviz_wrapper::OUTPUT_FILE_FILTER_PATTERNS.data(),
                                    nullptr);
                            if (outputPathChars != nullptr) {
                                std::filesystem::path outputFile = outputPathChars;
                                controller::getForegroundTasks().emplace("Export Connections with GraphViz", [outputFile, activeEditor, &engine](auto* progress) {
                                    *progress = .0f;
                                    auto graphvizCode = connection::visualization::generateGraphviz(engine.getConnections());
                                    *progress = .3f;
                                    graphvizCode.renderToFile(outputFile);
                                    if (outputFile.extension() == ".svg") {
                                        graphvizCode.deleteTmpFiles = false;
                                    }
                                    *progress = 1.f;
                                });
                            }
                        }
                    } else {
                        ImGui::BeginDisabled();
                        ImGui::Button("Export all Connections with GraphViz");
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                            ImGui::SetTooltip("GraphViz could not be detected");
                        }
                        ImGui::EndDisabled();
                    }

                    if (ImGui::Button(ICON_FA_DOWNLOAD " Export all Connections as .dot")) {
                        const char* filterPatterns = {".dot"};
                        char const* outputPathChars = tinyfd_saveFileDialog(
                                "Export Connection Data",
                                "connections.dot",
                                1,
                                &filterPatterns,
                                nullptr);
                        auto graphvizCode = connection::visualization::generateGraphviz(engine.getConnections());
                        graphvizCode.deleteTmpFiles = false;
                        std::ofstream file(outputPathChars);
                        file << graphvizCode.dotCode;
                    }

                    if (ImGui::Button(ICON_FA_CLIPBOARD_LIST " Export all Connections to CSV")) {
                        const char* filterPatterns = {".csv"};
                        char const* outputPathChars = tinyfd_saveFileDialog(
                                "Export Connection Data",
                                "connections.csv",
                                1,
                                &filterPatterns,
                                nullptr);
                        std::ofstream csv(outputPathChars);
                        for (const auto& [nA, listA]: engine.getConnections().getAdjacencyLists()) {
                            for (const auto& [nB, listB]: listA) {
                                for (const auto& edge: listB) {
                                    csv << fmt::format("{};{};", fmt::ptr(nA.get()), fmt::ptr(nB.get()));
                                    csv << fmt::format("{} {}\n", edge->connectorA->infoStr(), edge->connectorB->infoStr());
                                }
                            }
                        }
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            if (ImGui::BeginTabBar("##debugTabBar")) {
                drawGeneralTab();
                drawPerformanceTab();
                drawMeshesTab();
                drawCameraTab();
                drawConnectionTab();

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}
