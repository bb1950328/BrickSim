#include "../../config.h"
#include "../../controller.h"
#include "../../graphics/scene.h"
#include "../../types.h"
#include "../gui_internal.h"
#include <imgui_internal.h>
#include <memory>

#include "../../lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "../gui.h"
#include "spdlog/fmt/bundled/format.h"
#include "window_view3d.h"

namespace bricksim::gui::windows::view3d {
    std::shared_ptr<etree::Node> getNodeUnderCursor(const std::shared_ptr<graphics::Scene>& mainScene, const glm::svec2& currentCursorPos) {
        const auto elementIdUnderCursor = mainScene->getSelectionPixel(currentCursorPos);
        auto nodeUnderCursor = elementIdUnderCursor != 0
                                       ? mainScene->getMeshCollection().getElementById(elementIdUnderCursor)
                                       : nullptr;
        return nodeUnderCursor;
    }

    void draw(Data& data) {
        const auto& activeEditor = controller::getActiveEditor();
        ImGuiDockNode* lastDockNode = nullptr;
        for (auto& editor: controller::getEditors()) {
            const auto fileName = editor->getFilename();
            std::string windowTitle = fmt::format("{}{}{}###View3D{}", editor == activeEditor ? ICON_FA_EDIT " " : "", editor->isModified() ? "*" : "", fileName, fileName);
            bool open = true;
            ImGuiWindow* imGuiWindow = ImGui::FindWindowByName(windowTitle.c_str());
            if (imGuiWindow == nullptr) {
                if (lastDockNode != nullptr) {
                    ImGui::DockBuilderDockWindow(windowTitle.c_str(), lastDockNode->ID);
                }
            } else if (imGuiWindow->DockNode != nullptr) {
                lastDockNode = imGuiWindow->DockNode;
            }
            if (ImGui::Begin(windowTitle.c_str(), &open, ImGuiWindowFlags_NoScrollWithMouse)) {
                ImGui::BeginChild("3DRender");
                const ImVec2& regionAvail = ImGui::GetContentRegionAvail();
                const auto& scene = editor->getScene();
                scene->setImageSize({regionAvail.x, regionAvail.y});
                const ImVec2& windowPos = ImGui::GetWindowPos();
                const ImVec2& regionMin = ImGui::GetWindowContentRegionMin();
                const ImVec2& mousePos = ImGui::GetMousePos();
                const ImVec2& regionMax = ImGui::GetWindowContentRegionMax();
                const glm::svec2 relCursorPos = {
                        mousePos.x - windowPos.x - regionMin.x,
                        mousePos.y - windowPos.y - regionMin.y};
                bool isInWindow = (windowPos.x + regionMin.x <= mousePos.x
                                   && mousePos.x <= windowPos.x + regionMax.x
                                   && windowPos.y + regionMin.y <= mousePos.y
                                   && mousePos.y <= windowPos.y + regionMax.y);
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
                    const glm::svec2 currentCursorPos{mousePos.x, mousePos.y};
                    static bool lastLeftMouseDown = currentLeftMouseDown;
                    static bool lastMiddleMouseDown = currentMiddleMouseDown;
                    static bool lastRightMouseDown = currentRightMouseDown;
                    static glm::svec2 lastCursorPos = currentCursorPos;
                    static glm::svec2 totalDragDelta = {0, 0};

                    static DragMode dragMode = NOT_DRAGGING;

                    const glm::svec2 deltaCursorPos = currentCursorPos - lastCursorPos;
                    const bool currentlyAnyMouseDown = currentLeftMouseDown || currentMiddleMouseDown || currentRightMouseDown;
                    static bool lastAnyMouseDown = currentlyAnyMouseDown;

                    if (currentlyAnyMouseDown && dragMode == NOT_DRAGGING && (deltaCursorPos.x != 0 || deltaCursorPos.y != 0)) {
                        //drag just started
                        //todo handle multiple buttons pressed
                        if (currentRightMouseDown) {
                            dragMode = PAN_CAMERA;
                        } else if (currentMiddleMouseDown) {
                            //todo find something else that makes sense to drag with the middle mouse button
                            dragMode = ROTATE_CAMERA;
                        } else {
                            std::shared_ptr<etree::Node> nodeUnderCursor = getNodeUnderCursor(scene, relCursorPos);
                            if (nodeUnderCursor && editor->isNodeDraggable(nodeUnderCursor)) {
                                dragMode = DRAG_ELEMENT;
                                editor->startNodeDrag(nodeUnderCursor, relCursorPos);
                            } else {
                                dragMode = ROTATE_CAMERA;
                            }
                        }
                        totalDragDelta = {0, 0};
                    }

                    if (lastAnyMouseDown && !currentlyAnyMouseDown && dragMode == NOT_DRAGGING) {
                        //user just ended click without dragging
                        auto nodeUnderCursor = getNodeUnderCursor(scene, relCursorPos);
                        if (nodeUnderCursor) {
                            if (lastLeftMouseDown && editor->isNodeClickable(nodeUnderCursor)) {
                                editor->nodeClicked(nodeUnderCursor, imGuiIo.KeyCtrl, imGuiIo.KeyShift);
                            }
                            //todo add context menu when right click
                            //todo add something else when middle click
                        } else {
                            editor->nodeSelectNone();
                        }
                    }

                    if (dragMode != NOT_DRAGGING) {
                        if (currentlyAnyMouseDown) {
                            totalDragDelta += deltaCursorPos;
                        } else {
                            //dragging just stopped
                            if (dragMode == DRAG_ELEMENT) {
                                editor->endNodeDrag();
                            }
                            dragMode = NOT_DRAGGING;
                        }
                    }

                    auto& camera = editor->getCamera();
                    switch (dragMode) {
                        case NOT_DRAGGING: break;
                        case ROTATE_CAMERA:
                            camera->mouseRotate(deltaCursorPos);
                            break;
                        case PAN_CAMERA:
                            camera->mousePan(deltaCursorPos);
                            break;
                        case DRAG_ELEMENT:
                            editor->updateNodeDragDelta(totalDragDelta);
                            break;
                    }

                    auto lastScrollDeltaY = getLastScrollDeltaY();
                    if (std::abs(lastScrollDeltaY) > 0.01) {
                        camera->moveForwardBackward((float)lastScrollDeltaY);
                    }

                    lastLeftMouseDown = currentLeftMouseDown;
                    lastMiddleMouseDown = currentMiddleMouseDown;
                    lastRightMouseDown = currentRightMouseDown;
                    lastCursorPos = currentCursorPos;
                    lastAnyMouseDown = currentlyAnyMouseDown;
                }
                scene->setImageSize({regionAvail.x, regionAvail.y});
                ImTextureID texture3dView;
                glm::usvec2 imageSize;
                if (config::get(config::DISPLAY_SELECTION_BUFFER)) {
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
    }
}