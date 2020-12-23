//
// Created by bb1950328 on 15.11.2020.
//

#include "gui.h"
#include "../controller.h"

namespace gui {
    void windows::draw3dWindow(bool *show) {
        ImGui::Begin(ICON_FA_CUBES" 3D View", show, ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::BeginChild("3DRender");
            ImVec2 wsize = ImGui::GetContentRegionAvail();
            controller::set3dViewSize((unsigned int) wsize.x, (unsigned int) wsize.y);
            const ImVec2 &windowPos = ImGui::GetWindowPos();
            const ImVec2 &regionMin = ImGui::GetWindowContentRegionMin();
            const ImVec2 &mousePos = ImGui::GetMousePos();
            const ImVec2 &regionMax = ImGui::GetWindowContentRegionMax();
            bool isInWindow = (windowPos.x + regionMin.x <= mousePos.x
                               && mousePos.x <= windowPos.x + regionMax.x
                               && windowPos.y + regionMin.y <= mousePos.y
                               && mousePos.y <= windowPos.y + regionMax.y);
            static float lastDeltaXleft = 0, lastDeltaYleft = 0;
            static float lastDeltaXright = 0, lastDeltaYright = 0;
            static bool leftMouseDragging = false;
            //std::cout << ImGui::GetScrollX() << "\t" << ImGui::GetScrollY() << std::endl;
            if (isInWindow && ImGui::IsWindowFocused()) {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    leftMouseDragging = true;
                    const ImVec2 &leftBtnDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                    controller::getRenderer()->camera.mouseRotate(leftBtnDrag.x - lastDeltaXleft,
                                                                  (leftBtnDrag.y - lastDeltaYleft) * -1);
                    controller::getRenderer()->unrenderedChanges = true;
                    lastDeltaXleft = leftBtnDrag.x;
                    lastDeltaYleft = leftBtnDrag.y;
                } else {
                    lastDeltaXleft = 0;
                    lastDeltaYleft = 0;
                }
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                    const ImVec2 &rightBtnDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
                    controller::getRenderer()->camera.mousePan(rightBtnDrag.x - lastDeltaXright,
                                                               (rightBtnDrag.y - lastDeltaYright) * -1);
                    controller::getRenderer()->unrenderedChanges = true;
                    lastDeltaXright = rightBtnDrag.x;
                    lastDeltaYright = rightBtnDrag.y;
                } else {
                    lastDeltaXright = 0;
                    lastDeltaYright = 0;
                }
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && lastDeltaXleft == 0 && lastDeltaYleft == 0) {
                    if (!leftMouseDragging) {
                        auto relCursorPosX = ImGui::GetMousePos().x - ImGui::GetWindowPos().x - ImGui::GetWindowContentRegionMin().x;
                        auto relCursorPosY = ImGui::GetMousePos().y - ImGui::GetWindowPos().y - ImGui::GetWindowContentRegionMin().y;
                        //std::cout << "relCursorPos: " << relCursorPosX << ", " << relCursorPosY << std::endl;
                        //std::cout << "GetWindowPos: " << ImGui::GetWindowPos().x << ", " << ImGui::GetWindowPos().y << std::endl;
                        //std::cout << "GetMousePos: " << ImGui::GetMousePos().x << ", " << ImGui::GetMousePos().y << std::endl << std::endl;
                        const auto elementIdUnderMouse = controller::getRenderer()->getSelectionPixel(relCursorPosX, relCursorPosY);
                        if (elementIdUnderMouse == 0) {
                            controller::nodeSelectNone();
                        } else {
                            auto *clickedNode = controller::getRenderer()->meshCollection.getElementById(elementIdUnderMouse);
                            if (clickedNode != nullptr) {
                                if (ImGui::GetIO().KeyCtrl) {
                                    controller::nodeSelectAddRemove(clickedNode);
                                } else if (ImGui::GetIO().KeyShift) {
                                    controller::nodeSelectUntil(clickedNode);
                                } else {
                                    controller::nodeSelectSet(clickedNode);
                                }
                                //std::cout << clickedNode->displayName << std::endl;
                            }
                        }
                    }
                    leftMouseDragging = false;
                }
                auto lastScrollDeltaY = getLastScrollDeltaY();
                if (std::abs(lastScrollDeltaY) > 0.01) {
                    controller::getRenderer()->camera.moveForwardBackward((float) lastScrollDeltaY);
                    controller::getRenderer()->unrenderedChanges = true;
                }
            }
            auto texture3dView = (ImTextureID) (config::getBool(config::DISPLAY_SELECTION_BUFFER)
                                                ? controller::getRenderer()->selectionTextureColorbuffer
                                                : controller::getRenderer()->imageTextureColorbuffer);
            ImGui::ImageButton(texture3dView, wsize, ImVec2(0, 1), ImVec2(1, 0), 0);
            ImGui::EndChild();
        }
        ImGui::End();
    }
}