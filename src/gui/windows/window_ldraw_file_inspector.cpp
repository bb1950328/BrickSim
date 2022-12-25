#include "window_ldraw_file_inspector.h"
#include "../../connection/connector_data_provider.h"
#include "../../element_tree.h"
#include "../../graphics/connection_visualization.h"
#include "../../ldr/file_repo.h"
#include "../../ldr/file_writer.h"
#include "../../ldr/shadow_file_repo.h"
#include "../gui.h"
#include "../gui_internal.h"
#include "glm/gtx/string_cast.hpp"
#include "imgui.h"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/spdlog.h"
#include <sstream>

namespace bricksim::gui::windows::ldraw_file_inspector {
    namespace {
        std::shared_ptr<ldr::File> currentFile = nullptr;
        std::string content;
        std::string shadowContent;

        void currentFileChanged() {
            if (currentFile != nullptr) {
                std::stringstream sstr;
                ldr::writeFile(currentFile, sstr, currentFile->metaInfo.name);
                content = sstr.str();
                try {
                    const auto relPath = bricksim::ldr::file_repo::FileRepo::getPathRelativeToBase(currentFile->metaInfo.type, currentFile->metaInfo.name);
                    shadowContent = ldr::file_repo::getShadowFileRepo().getContent(relPath).value_or("");
                } catch (std::invalid_argument) {
                    shadowContent = "";
                }
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
            if (ImGui::BeginChild("##snapConnectorTree")) {
                const auto &connectors = connection::getConnectorsOfPart(currentFile->metaInfo.name);
                char *nodeId = 0;
                for (const auto &item: connectors) {
                    const auto clipConn = std::dynamic_pointer_cast<connection::ClipConnector>(item);
                    const auto cylConn = std::dynamic_pointer_cast<connection::CylindricalConnector>(item);
                    const auto fgrConn = std::dynamic_pointer_cast<connection::FingerConnector>(item);
                    const auto genConn = std::dynamic_pointer_cast<connection::GenericConnector>(item);
                    std::string name;
                    if (clipConn != nullptr) {
                        name = "Clip";
                    } else if (cylConn != nullptr) {
                        name = fmt::format("Cylinder {:g} {:g} {:g} {}", cylConn->start.x, cylConn->start.y, cylConn->start.z, magic_enum::enum_name(cylConn->gender));
                    } else if (fgrConn != nullptr) {
                        name = "Finger";
                    } else if (genConn != nullptr) {
                        name = "Generic";
                    }
                    if (ImGui::TreeNode((void*)(nodeId++), "%s", name.c_str())) {
                        ImGui::BulletText("start=%s", glm::to_string(item->start).c_str());
                        if (clipConn != nullptr) {
                            ImGui::BulletText("direction=%s", glm::to_string(clipConn->direction).c_str());
                            ImGui::BulletText("radius=%f", clipConn->radius);
                            ImGui::BulletText("width=%f", clipConn->width);
                            ImGui::BulletText("slide=%s", std::to_string(clipConn->slide).c_str());
                        } else if (cylConn != nullptr) {
                            ImGui::BulletText("direction=%s", glm::to_string(cylConn->direction).c_str());
                            ImGui::BulletText("gender=%s", magic_enum::enum_name(cylConn->gender).data());
                            if (ImGui::TreeNode("Parts")) {
                                for (const auto &part: cylConn->parts) {
                                    ImGui::BulletText("type=%s flexibleRadius=%s radius=%f length=%f",
                                                      magic_enum::enum_name(part.type).data(),
                                                      std::to_string(part.flexibleRadius).c_str(),
                                                      part.radius,
                                                      part.length);
                                }
                                ImGui::TreePop();
                            }
                            ImGui::BulletText("openStart=%s", std::to_string(cylConn->openStart).c_str());
                            ImGui::BulletText("openEnd=%s", std::to_string(cylConn->openEnd).c_str());
                            ImGui::BulletText("slide=%s", std::to_string(cylConn->slide).c_str());
                        } else if (fgrConn != nullptr) {
                            ImGui::BulletText("direction=%s", glm::to_string(fgrConn->direction).c_str());
                            ImGui::BulletText("gender=%s", magic_enum::enum_name(fgrConn->firstFingerGender).data());
                            ImGui::BulletText("radius=%f", fgrConn->radius);
                            if (ImGui::TreeNode("Finger Widths")) {
                                for (const auto &width: fgrConn->fingerWidths) {
                                    ImGui::BulletText("%f", width);
                                }
                                ImGui::TreePop();
                            }
                        } else if (genConn != nullptr) {
                            ImGui::BulletText("gender=%s", magic_enum::enum_name(genConn->gender).data());
                            if (std::holds_alternative<connection::BoundingPnt>(genConn->bounding)) {
                                const auto &bounding = std::get<connection::BoundingPnt>(genConn->bounding);
                                ImGui::BulletText("bounding=point");
                            } else if (std::holds_alternative<connection::BoundingBox>(genConn->bounding)) {
                                const auto &bounding = std::get<connection::BoundingBox>(genConn->bounding);
                                ImGui::BulletText("bounding=box");
                                ImGui::BulletText("x=%f", bounding.x);
                                ImGui::BulletText("y=%f", bounding.y);
                                ImGui::BulletText("z=%f", bounding.z);
                            } else if (std::holds_alternative<connection::BoundingCube>(genConn->bounding)) {
                                const auto &bounding = std::get<connection::BoundingCube>(genConn->bounding);
                                ImGui::BulletText("bounding=cube");
                                ImGui::BulletText("size=%f", bounding.size);
                            } else if (std::holds_alternative<connection::BoundingCyl>(genConn->bounding)) {
                                const auto &bounding = std::get<connection::BoundingCyl>(genConn->bounding);
                                ImGui::BulletText("bounding=cyl");
                                ImGui::BulletText("radius=%f", bounding.radius);
                                ImGui::BulletText("length=%f", bounding.length);
                            } else if (std::holds_alternative<connection::BoundingSph>(genConn->bounding)) {
                                const auto &bounding = std::get<connection::BoundingSph>(genConn->bounding);
                                ImGui::BulletText("bounding=sph");
                                ImGui::BulletText("radius=%f", bounding.radius);
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::EndChild();
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

            if (dragging && (cursorDelta.x != 0 || cursorDelta.y != 0)) {
                camera->mouseRotate(cursorDelta);
            }

            const auto lastScrollDeltaY = getLastScrollDeltaY();
            if (std::abs(lastScrollDeltaY) > 0.01 && nowFocussedAndHovered) {
                camera->moveForwardBackward((float) lastScrollDeltaY);
            }

            lastLeftDown = nowLeftDown;
            lastRelCursorPos = nowRelCursorPos;

            const auto visualizationImg = graphics::connection_visualization::getImage();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::ImageButton("connectionVisualization",
                               gui_internal::convertTextureId(visualizationImg),
                               imgButtonSize,
                               {0, 1},
                               {1, 0});
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
            static std::array<bool, magic_enum::enum_count<ldr::FileType>()> showTypes;

            if (static bool first = true; first) {
                first = false;
                std::fill(showTypes.begin(), showTypes.end(), true);
            }
            char partName[256] = {0};
            ImGui::InputText("Part Name", partName, sizeof(partName), ImGuiInputTextFlags_CallbackAlways,
                             partNameInputCallback);

            if (ImGui::BeginTable("##File selector", 2, ImGuiTableFlags_None)) {
                ImGui::TableSetupColumn("##filters", ImGuiTableColumnFlags_NoReorder
                                                     | ImGuiTableColumnFlags_NoResize
                                                     | ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("##listbox", ImGuiTableColumnFlags_NoReorder
                                                     | ImGuiTableColumnFlags_NoResize);
                ImGui::TableNextColumn();
                for (int i = 0; i < magic_enum::enum_count<ldr::FileType>(); ++i) {
                    const auto type = static_cast<const ldr::FileType>(i);
                    const auto typeName = magic_enum::enum_name<ldr::FileType>(type);
                    ImGui::Checkbox(typeName.data(), &showTypes[i]);
                }

                ImGui::TableNextColumn();

                ImGui::PushItemWidth(-1.f);
                if (ImGui::BeginListBox("##All Files")) {
                    for (const auto &item: ldr::file_repo::get().getAllFilesInMemory()) {
                        if (showTypes[item.second.second->metaInfo.type]) {
                            const auto text = fmt::format("{}: {}", item.first, item.second.second->metaInfo.title);
                            if (ImGui::Selectable(text.c_str(), item.second.second == currentFile)) {
                                setCurrentFile(item.second.second);
                            }
                        }
                    }
                    ImGui::EndListBox();
                }
                ImGui::PopItemWidth();

                ImGui::EndTable();
            }

            if (currentFile != nullptr) {
                ImGui::Separator();

                ImGui::Text("Inspecting %s: %s", currentFile->metaInfo.name.c_str(),
                            currentFile->metaInfo.title.c_str());

                if (ImGui::BeginTabBar("##fileInspectorTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
                    if (ImGui::BeginTabItem("Raw Content")) {
                        if (ImGui::BeginChild("Content", ImVec2(0, 0), true, ImGuiWindowFlags_None)) {
                            ImGui::TextUnformatted(content.c_str());
                        }
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Shadow Content")) {
                        if (ImGui::BeginChild("shadowContent", ImVec2(0, 0), true, ImGuiWindowFlags_None)) {
                            ImGui::TextUnformatted(shadowContent.c_str());
                        }
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Meta Snap Info")) {
                        static std::weak_ptr<connection::ldcad_snap_meta::MetaCommand> currentlySelected;
                        showSnapLineNodes(currentFile, currentlySelected);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("BrickSim Snap Info")) {
                        showBrickSimConnectionVisualization();
                        ImGui::SameLine();
                        showBrickSimSnapConnectorTree();

                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Meta Info")) {
                        showMetaInfo();
                        ImGui::EndTabItem();
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
