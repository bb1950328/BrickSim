#include "window_ldraw_file_inspector.h"
#include "../../element_tree.h"
#include "../../ldr/file_repo.h"
#include "../../ldr/file_writer.h"
#include "imgui.h"
#include "spdlog/fmt/bundled/format.h"
#include "../../connection/connector_data_provider.h"
#include "../../graphics/connection_visualization.h"
#include "../gui_internal.h"
#include "spdlog/spdlog.h"
#include "../gui.h"
#include <sstream>

namespace bricksim::gui::windows::ldraw_file_inspector {
    namespace {
        std::shared_ptr<ldr::File> currentFile = nullptr;
        std::string content;

        void currentFileChanged() {
            if (currentFile != nullptr) {
                std::stringstream sstr;
                ldr::writeFile(currentFile, sstr, currentFile->metaInfo.name);
                content = sstr.str();
            } else {
                content = "";
            }
        }

        int partNameInputCallback(ImGuiInputTextCallbackData *data) {
            auto &fileRepo = ldr::file_repo::get();
            if (fileRepo.hasFileCached(data->Buf)) {
                setCurrentFile(fileRepo.getFile(data->Buf));
            } else {
                const auto extendedName = std::string(data->Buf) + ".dat";
                if (fileRepo.hasFileCached(extendedName)) {
                    const auto lengthBefore = data->BufTextLen;
                    data->InsertChars(lengthBefore, ".dat");
                    data->CursorPos = lengthBefore;
                    setCurrentFile(fileRepo.getFile(extendedName));
                } else {
                    setCurrentFile(nullptr);
                }
            }
            return 0;
        }

        void showSnapLineNodes(const std::shared_ptr<ldr::File> &file,
                               std::weak_ptr<connection::ldcad_snap_meta::MetaCommand> &currentlySelected) {
            for (const auto &item: file->ldcadSnapMetas) {
                auto flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (currentlySelected.lock() == item) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }
                ImGui::TreeNodeEx(reinterpret_cast<const void *>(item.get()), flags, "%s", item->to_string().c_str());
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    currentlySelected = item;
                }
            }
            for (const auto &item: file->elements) {
                if (item->getType() == 1) {
                    const auto subfileRef = std::dynamic_pointer_cast<ldr::SubfileReference>(item);
                    const auto subfile = subfileRef->getFile();
                    if (!subfile->ldcadSnapMetas.empty()) {
                        if (ImGui::TreeNode(subfileRef.get(), "%s", subfileRef->filename.c_str())) {
                            showSnapLineNodes(subfile, currentlySelected);
                            ImGui::TreePop();
                        }
                    }
                }
            }
        }

        void showBrickSimSnapConnectorTree() {
            const auto connectors = connection::getConnectorsOfPart(currentFile->metaInfo.name);
            for (const auto &item: connectors) {
                const auto clipConn = std::dynamic_pointer_cast<connection::ClipConnector>(item);
                const auto cylConn = std::dynamic_pointer_cast<connection::CylindricalConnector>(item);
                const auto fgrConn = std::dynamic_pointer_cast<connection::FingerConnector>(item);
                const auto genConn = std::dynamic_pointer_cast<connection::GenericConnector>(item);
                const char *name;
                if (clipConn != nullptr) {
                    name = "Clip";
                } else if (cylConn != nullptr) {
                    name = "Cylinder";
                } else if (fgrConn != nullptr) {
                    name = "Finger";
                } else if (genConn != nullptr) {
                    name = "Generic";
                }
                if (ImGui::TreeNode(item.get(), "%s", name)) {
                    if (clipConn != nullptr) {
                        ImGui::BulletText("AA");
                        ImGui::BulletText("AA2");
                    } else if (cylConn != nullptr) {
                        ImGui::BulletText("BB");
                        ImGui::BulletText("BB2");
                    } else if (fgrConn != nullptr) {
                        ImGui::BulletText("CC");
                        ImGui::BulletText("CC3");
                    } else if (genConn != nullptr) {
                        ImGui::BulletText("DD");
                        ImGui::BulletText("DD4");
                    }
                    ImGui::TreePop();
                }
            }
        }

        void showBrickSimConnectionVisualization() {
            constexpr ImVec2 imgButtonSize(512, 512);
            graphics::connection_visualization::initializeIfNeeded();
            graphics::connection_visualization::setVisualizedPart(currentFile->metaInfo.name);
            const auto camera = graphics::connection_visualization::getCamera();

            const ImVec2 &windowPos = ImGui::GetWindowPos();
            const ImVec2 &imgPos = ImGui::GetCursorPos();
            const ImVec2 &mousePos = ImGui::GetMousePos();
            const ImGuiIO &imGuiIo = ImGui::GetIO();
            const glm::svec2 nowRelCursorPos = {
                    mousePos.x - windowPos.x - imgPos.x,
                    mousePos.y - windowPos.y - imgPos.y};
            static glm::svec2 lastRelCursorPos = nowRelCursorPos;
            const glm::svec2 cursorDelta = nowRelCursorPos - lastRelCursorPos;
            static bool lastLeftDown = false;

            const bool nowFocussedAndHovered = ImGui::IsWindowFocused(ImGuiFocusedFlags_None)
                                               && 0 <= nowRelCursorPos.x && nowRelCursorPos.x < imgButtonSize.x
                                               && 0 <= nowRelCursorPos.y && nowRelCursorPos.y < imgButtonSize.y;
            const bool nowLeftDown = imGuiIo.MouseDown[ImGuiMouseButton_Left];

            static bool dragging = false;
            if (!lastLeftDown && nowLeftDown && nowFocussedAndHovered) {
                dragging = true;
            } else if (lastLeftDown && !nowLeftDown) {
                dragging = false;
            }

            spdlog::debug("lastLeft={}, nowLeft={}, nowFocus={}, dragging={}, lastPos={};{}, nowPos={};{}, delta={};{}",
                          lastLeftDown, nowLeftDown, nowFocussedAndHovered, dragging,
                          lastRelCursorPos.x, lastRelCursorPos.y,
                          nowRelCursorPos.x, nowRelCursorPos.y,
                          cursorDelta.x, cursorDelta.y);
            if (dragging && (cursorDelta.x != 0 || cursorDelta.y != 0)) {
                camera->mouseRotate(cursorDelta);
            }

            const auto lastScrollDeltaY = getLastScrollDeltaY();
            if (std::abs(lastScrollDeltaY) > 0.01) {
                camera->moveForwardBackward((float) lastScrollDeltaY);
            }

            lastLeftDown = nowLeftDown;
            lastRelCursorPos = nowRelCursorPos;

            const auto visualizationImg = graphics::connection_visualization::getImage();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::ImageButton("connectionVisualization",
                               gui_internal::convertTextureId(visualizationImg),
                               imgButtonSize);
            //todo handle mouse drag (update camera)
            ImGui::PopStyleVar();
        }

        void showMetaInfo() {
            const auto &metaInfo = currentFile->metaInfo;

            if (ImGui::BeginTable("##metaInfoTable", 2)) {
                constexpr auto rowStart = [](const char *const name) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", name);
                    ImGui::TableNextColumn();
                };

                ImGui::TableSetupColumn("Attribute", ImGuiTableColumnFlags_WidthFixed,
                                        ImGui::GetFontSize() * 6);
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();

                rowStart("Title");
                ImGui::Text("%s", metaInfo.title.c_str());

                rowStart("Name");
                ImGui::Text("%s", metaInfo.name.c_str());

                rowStart("Author");
                ImGui::Text("%s", metaInfo.author.c_str());

                rowStart("Keywords");
                for (const auto &item: metaInfo.keywords) {
                    ImGui::BulletText("%s", item.c_str());
                }

                rowStart("History");
                for (const auto &item: metaInfo.history) {
                    ImGui::BulletText("%s", item.c_str());
                }

                rowStart("License");
                ImGui::Text("%s", metaInfo.license.c_str());

                rowStart("Theme");
                ImGui::Text("%s", metaInfo.theme.c_str());

                rowStart("Category");
                ImGui::Text("%s", metaInfo.headerCategory.value_or("").c_str());

                rowStart("File Type");
                ImGui::Text("%s", std::string(magic_enum::enum_name(metaInfo.type)).c_str());

                ImGui::EndTable();
            }

            ImGui::EndTabItem();

            /*

            ImGui::Separator();

            static bool explicitCategory;
            if (selectedEditorLocked != lastSelectedEditor.lock()) {
                explicitCategory = metaInfo.headerCategory.has_value();
            }
            if (ImGui::Checkbox("Explicit Category", &explicitCategory)) {
                if (explicitCategory && !metaInfo.headerCategory.has_value()) {
                    metaInfo.headerCategory = metaInfo.getCategory();
                } else if (!explicitCategory && metaInfo.headerCategory.has_value()) {
                    metaInfo.headerCategory = std::nullopt;
                }
            }
            if (explicitCategory) {
                ImGui::InputText("Category", &metaInfo.headerCategory.value());
            } else {
                ImGui::BeginDisabled();
                static char zeroChar = 0;
                ImGui::InputText("Category", &zeroChar, 1);
                ImGui::EndDisabled();
            }*/
        }
    }

    void setCurrentFile(const std::shared_ptr<ldr::File> &newFile) {
        if (currentFile != newFile) {
            currentFile = newFile;
            currentFileChanged();
        }
    }

    void draw(Data &data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            char partName[256] = {0};
            ImGui::InputText("Part Name", partName, sizeof(partName), ImGuiInputTextFlags_CallbackAlways,
                             partNameInputCallback);

            if (ImGui::BeginListBox("All Files")) {
                for (const auto &item: ldr::file_repo::get().getAllFilesInMemory()) {
                    const auto text = fmt::format("{}: {}", item.first, item.second.second->metaInfo.title);
                    if (ImGui::Selectable(text.c_str(), item.second.second == currentFile)) {
                        setCurrentFile(item.second.second);
                    }
                }
                ImGui::EndListBox();
            }

            if (currentFile != nullptr) {
                ImGui::Separator();

                ImGui::Text("Inspecting %s: %s", currentFile->metaInfo.name.c_str(),
                            currentFile->metaInfo.title.c_str());

                if (ImGui::BeginTabBar("##fileInspectorTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
                    if (ImGui::BeginTabItem("Raw Content")) {
                        if (ImGui::BeginChild("Content", ImVec2(0, 0), true, ImGuiWindowFlags_None)) {
                            ImGui::TextUnformatted(content.c_str());
                            ImGui::EndChild();
                        }
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Meta Snap Info")) {
                        static std::weak_ptr<connection::ldcad_snap_meta::MetaCommand> currentlySelected;
                        showSnapLineNodes(currentFile, currentlySelected);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("BrickSim Snap Info")) {
                        showBrickSimConnectionVisualization();

                        showBrickSimSnapConnectorTree();

                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Meta Info")) {
                        showMetaInfo();
                    }
                    ImGui::EndTabBar();
                }

            } else {
                ImGui::Text("No file named \"%s\" in memory.", partName);
            }
        }
        ImGui::End();
    }
}
