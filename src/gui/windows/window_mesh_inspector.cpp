#include <imgui.h>
#include <memory>
#include <utility>
#include "window_mesh_inspector.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "../gui.h"
#include <sstream>

namespace bricksim::gui::windows::mesh_inspector {
    namespace {
        std::shared_ptr<mesh::Mesh> currentlyInspectingMesh;

        void drawColorLabel(const ldr::ColorReference &colorRef) {
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

        void drawTriangleVerticesTab(std::shared_ptr<mesh::Mesh> &mesh) {
            if (ImGui::BeginTabItem("Triangle Vertices")) {
                const char *items[] = {"Vertices & Indices separate", "\"Inline\" Indices"};
                static int currentMode = 0;
                ImGui::Combo("Mode", &currentMode, items, std::size(items));

                static ldr::ColorReference currentColor = ldr::color_repo::NO_COLOR_CODE;
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
                    copyIndicesToClipboard = ImGui::Button(ICON_FA_CLIPBOARD
                                                           " Copy Indices");
                }
                std::stringstream toClipboard;

                if (copyVerticesToClipboard) {
                    toClipboard << "pos.x;pos.y;pos.z;pos.w;normal.x;normal.y;normal.z" << std::endl;
                }

                auto drawVertexRow = [&copyVerticesToClipboard, &toClipboard](unsigned int index, const mesh::TriangleVertex &vertex) {
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

        void drawInstancesTab(std::shared_ptr<mesh::Mesh> &mesh) {
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

        void drawGeneralTab(std::shared_ptr<mesh::Mesh> &mesh) {
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

    void draw(Data &data) {
        if (ImGui::Begin(data.name, &data.visible, ImGuiWindowFlags_None)) {
            if (currentlyInspectingMesh == nullptr) {
                ImGui::Text("Currently there's no mesh selected for inspection. Select one in the debug window.");
            } else if (ImGui::BeginTabBar("##meshInspectorTabs")) {
                drawGeneralTab(currentlyInspectingMesh);
                drawInstancesTab(currentlyInspectingMesh);
                drawTriangleVerticesTab(currentlyInspectingMesh);

                ImGui::EndTabBar();
            }
            if (ImGui::Button(ICON_FA_WINDOW_CLOSE" Close")) {
                data.visible = false;
            }
        }
        ImGui::End();
    }

    void setCurrentlyInspectingMesh(std::shared_ptr<mesh::Mesh> mesh) {
        currentlyInspectingMesh = std::move(mesh);
    }
}