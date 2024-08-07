#include "../../config/read.h"
#include "../../controller.h"
#include "../../editor/tools.h"
#include "../../graphics/scene.h"
#include "../../types.h"
#include "../gui_internal.h"
#include <imgui_internal.h>
#include <memory>

#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../context_menu/node_context_menu.h"
#include "../gui.h"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/spdlog.h"
#include "window_view3d.h"

namespace bricksim::gui::windows::view3d {
    std::shared_ptr<etree::Node> getNodeUnderCursor(const std::shared_ptr<graphics::Scene>& mainScene, const glm::svec2& currentCursorPos) {
        const auto elementIdUnderCursor = mainScene->getSelectionPixel(currentCursorPos);
        auto nodeUnderCursor = elementIdUnderCursor != 0
                                   ? mainScene->getMeshCollection().getElementById(elementIdUnderCursor)
                                   : nullptr;
        return nodeUnderCursor;
    }

    void setCursor(const std::shared_ptr<Editor>& editor) {
        const auto& toolData = tools::getActiveToolData();
        auto& cursor = getCursorHandler().getIconCursor(toolData.icon);
        cursor.activate(getWindow());
    }

    void draw(Data& data) {
        ImGuiDockNode* lastDockNode = nullptr;
        bool windowInfoCollected = false;
        bool cursorSet = false;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        for (auto& editor: controller::getEditors()) {
            const bool isActiveEditor = editor->isActive();
            const auto windowTitle = editor->getDisplayName();
            bool open = true;
            ImGuiWindow* imGuiWindow = ImGui::FindWindowByName(windowTitle.c_str());
            if (imGuiWindow == nullptr) {
                if (lastDockNode != nullptr) {
                    ImGui::DockBuilderDockWindow(windowTitle.c_str(), lastDockNode->ID);
                }
            } else if (imGuiWindow->DockNode != nullptr) {
                lastDockNode = imGuiWindow->DockNode;
            }
            if (isActiveEditor) {
                ImGui::PushStyleColor(ImGuiCol_Text, COLOR_ACTIVE_EDITOR);
            }
            const auto windowOpen = ImGui::Begin(windowTitle.c_str(), nullptr, ImGuiWindowFlags_NoScrollWithMouse);
            if (isActiveEditor) {
                ImGui::PopStyleColor();
            }
            if (windowOpen) {
                if (!windowInfoCollected) {
                    collectWindowInfo(data.id);
                    windowInfoCollected = true;
                }
                ImGui::BeginChild("3DRender");
                const glm::svec2 regionAvail = glm::vec2(ImGui::GetContentRegionAvail());
                const auto& scene = editor->getScene();
                scene->setImageSize({regionAvail.x, regionAvail.y});
                const ImVec2& windowPos = ImGui::GetWindowPos();
                const ImVec2& mousePos = ImGui::GetMousePos();
                const glm::svec2 relMousePos = glm::vec2(ImGui::GetMousePos())-glm::vec2(ImGui::GetCursorStartPos())-glm::vec2(ImGui::GetWindowPos());
                bool isInWindow = 0 <= relMousePos.x && relMousePos.x <= regionAvail.x
                                  && 0 <= relMousePos.y && relMousePos.y <= regionAvail.y;
                if (ImGui::IsWindowHovered(ImGuiHoveredFlags_None)) {
                    setCursor(editor);
                    cursorSet = true;
                }
                enum DragMode {
                    NOT_DRAGGING,
                    ROTATE_CAMERA,
                    PAN_CAMERA,
                    DRAG_ELEMENT,
                };
                static bool lastIsWindowFocused = false;
                bool currentIsWindowFocused = ImGui::IsWindowFocused();
                if (!lastIsWindowFocused && currentIsWindowFocused) {
                    controller::setActiveEditor(editor);
                }
                if (isInWindow && currentIsWindowFocused) {
                    const ImGuiIO& imGuiIo = ImGui::GetIO();

                    const bool currentLeftMouseDown = imGuiIo.MouseDown[ImGuiMouseButton_Left];
                    const bool currentMiddleMouseDown = imGuiIo.MouseDown[ImGuiMouseButton_Middle];
                    const bool currentRightMouseDown = imGuiIo.MouseDown[ImGuiMouseButton_Right];
                    static bool lastLeftMouseDown = currentLeftMouseDown;
                    static bool lastMiddleMouseDown = currentMiddleMouseDown;
                    static bool lastRightMouseDown = currentRightMouseDown;
                    static glm::svec2 lastRelMousePos = relMousePos;
                    static glm::svec2 totalDragDelta = {0, 0};

                    static DragMode dragMode = NOT_DRAGGING;

                    if (!lastIsWindowFocused) {
                        lastRelMousePos = relMousePos;
                    }

                    const glm::svec2 deltaCursorPos = relMousePos - lastRelMousePos;
                    const bool currentlyAnyMouseDown = currentLeftMouseDown || currentMiddleMouseDown || currentRightMouseDown;
                    static bool lastAnyMouseDown = currentlyAnyMouseDown;

                    editor->updateCursorPos(relMousePos);

                    if (currentlyAnyMouseDown && dragMode == NOT_DRAGGING && (deltaCursorPos.x != 0 || deltaCursorPos.y != 0)) {
                        //drag just started
                        //todo handle multiple buttons pressed
                        if (currentRightMouseDown) {
                            dragMode = PAN_CAMERA;
                        } else if (currentMiddleMouseDown) {
                            //todo find something else that makes sense to drag with the middle mouse button
                            dragMode = ROTATE_CAMERA;
                        } else {
                            std::shared_ptr<etree::Node> nodeUnderCursor = getNodeUnderCursor(scene, relMousePos);
                            if (nodeUnderCursor && editor->getSelectedNodes().contains(nodeUnderCursor)) {
                                dragMode = DRAG_ELEMENT;
                                editor->startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType::TRANSLATE);
                            } else {
                                dragMode = ROTATE_CAMERA;
                            }
                        }
                        totalDragDelta = {0, 0};
                    }

                    if (lastAnyMouseDown && !currentlyAnyMouseDown && dragMode == NOT_DRAGGING) {
                        editor->endNodeTransformation();
                        //user just ended click without dragging
                        auto nodeUnderCursor = getNodeUnderCursor(scene, relMousePos);
                        if (nodeUnderCursor) {
                            if (lastLeftMouseDown || lastRightMouseDown) {
                                tools::getActiveToolData().handleNodeClicked->operator()(editor, nodeUnderCursor, imGuiIo.KeyCtrl, imGuiIo.KeyShift);
                            }
                            if (lastRightMouseDown) {
                                editor->openContextMenuNodeSelectedOrClicked(nodeUnderCursor);
                            }
                            //todo add something useful when middle click
                        } else {
                            tools::getActiveToolData().handleNodeClicked->operator()(editor, nullptr, imGuiIo.KeyCtrl, imGuiIo.KeyShift);
                            if (lastRightMouseDown) {
                                editor->openContextMenuNoNode();
                            }
                        }
                    }

                    if (dragMode != NOT_DRAGGING) {
                        if (currentlyAnyMouseDown) {
                            totalDragDelta += deltaCursorPos;
                        } else {
                            //dragging just stopped
                            dragMode = NOT_DRAGGING;
                        }
                    }

                    auto& camera = editor->getCamera();
                    switch (dragMode) {
                        case NOT_DRAGGING:
                            break;
                        case ROTATE_CAMERA:
                            camera->mouseRotate(deltaCursorPos);
                            break;
                        case PAN_CAMERA:
                            camera->mousePan(deltaCursorPos);
                            break;
                        case DRAG_ELEMENT:
                            break;
                    }

                    auto lastScrollDeltaY = getLastScrollDeltaY();
                    if (std::abs(lastScrollDeltaY) > 0.01) {
                        camera->moveForwardBackward((float)lastScrollDeltaY);
                    }

                    lastLeftMouseDown = currentLeftMouseDown;
                    lastMiddleMouseDown = currentMiddleMouseDown;
                    lastRightMouseDown = currentRightMouseDown;
                    lastRelMousePos = relMousePos;
                    lastAnyMouseDown = currentlyAnyMouseDown;
                } else {
                    editor->updateCursorPos(std::nullopt);
                }
                scene->setImageSize({regionAvail.x, regionAvail.y});
                ImTextureID texture3dView;
                glm::usvec2 imageSize;
                if (config::get().graphics.debug.displaySelectionBuffer) {
                    scene->getSelectionPixel({0, 0});//to update image
                    const auto& image = scene->getSelectionImage().value();
                    imageSize = image.getSize();
                    texture3dView = gui_internal::convertTextureId(image.getTexBO());
                } else {
                    const auto& image = scene->getImage();
                    texture3dView = gui_internal::convertTextureId(image.getTexBO());
                    imageSize = image.getSize();
                }
                //ImGui::ImageButton(texture3dView, ImVec2(image.getSize().x, image.getSize().y), ImVec2(0, 1), ImVec2(1, 0), 0);
                ImGui::Image(texture3dView, ImVec2(imageSize.x, imageSize.y), ImVec2(0, 1), ImVec2(1, 0));
                //spdlog::error("main texture {}", image.getTexBO());

                lastIsWindowFocused = currentIsWindowFocused;

                ImGui::EndChild();
            }
            ImGui::End();
        }
        ImGui::PopStyleVar();
        if (!cursorSet) {
            getCursorHandler().enableImGuiCursor();
        }
    }
}
