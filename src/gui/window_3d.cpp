

#include "gui.h"
#include "../controller.h"
#include "gui_internal.h"
#include "../config.h"

namespace gui {
    void windows::draw3dWindow(bool *show) {
        ImGui::Begin(WINDOW_NAME_3D_VIEW, show, ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::BeginChild("3DRender");
            const ImVec2 &regionAvail = ImGui::GetContentRegionAvail();
            controller::set3dViewSize((unsigned int) regionAvail.x, (unsigned int) regionAvail.y);
            if (ImGui::IsWindowFocused()) {
                const ImVec2 &windowPos = ImGui::GetWindowPos();
                const ImVec2 &regionMin = ImGui::GetWindowContentRegionMin();
                const ImVec2 &mousePos = ImGui::GetMousePos();
                const ImVec2 &regionMax = ImGui::GetWindowContentRegionMax();
                bool isInWindow = (windowPos.x + regionMin.x <= mousePos.x
                                   && mousePos.x <= windowPos.x + regionMax.x
                                   && windowPos.y + regionMin.y <= mousePos.y
                                   && mousePos.y <= windowPos.y + regionMax.y);
                if (isInWindow) {
                    const ImGuiIO &imGuiIo = ImGui::GetIO();
                    static bool lastLeftDown = false;
                    static bool lastRightDown = false;
                    static int lastCursorX = 0, lastCursorY = 0;
                    static bool isDragging = false;
                    const bool nowLeftDown = imGuiIo.MouseDown[ImGuiMouseButton_Left];
                    const bool nowRightDown = imGuiIo.MouseDown[ImGuiMouseButton_Right];
                    const int nowCursorX = (int) mousePos.x, nowCursorY = (int) mousePos.y;
                    const auto deltaX = nowCursorX - lastCursorX;
                    const auto deltaY = nowCursorY - lastCursorY;
                    const bool mouseHasMoved = deltaX != 0 || deltaY != 0;
                    isDragging = isDragging || ((nowLeftDown || nowRightDown) && mouseHasMoved);

                    if (isDragging && mouseHasMoved) {
                        if (lastLeftDown && nowLeftDown) {
                            controller::getMainSceneCamera()->mouseRotate(deltaX, deltaY);
                        }
                        if (lastRightDown && nowRightDown) {
                            controller::getMainSceneCamera()->mousePan(deltaX, deltaY);
                        }
                    }
                    if ((lastLeftDown && !nowLeftDown) && !isDragging) {
                        const auto relCursorPosX = mousePos.x - windowPos.x - regionMin.x;
                        const auto relCursorPosY = mousePos.y - windowPos.y - regionMin.y;
                        const auto elementIdUnderMouse = controller::getMainScene()->getSelectionPixel(relCursorPosX, relCursorPosY);
                        if (elementIdUnderMouse == 0) {
                            controller::nodeSelectNone();
                        } else {
                            auto clickedNode = controller::getMainScene()->getMeshCollection().getElementById(elementIdUnderMouse);
                            if (clickedNode != nullptr) {
                                if (imGuiIo.KeyCtrl) {
                                    controller::nodeSelectAddRemove(clickedNode);
                                } else if (imGuiIo.KeyShift) {
                                    controller::nodeSelectUntil(clickedNode);
                                } else {
                                    controller::nodeSelectSet(clickedNode);
                                }
                            }
                        }
                    }
                    if (!nowLeftDown && !nowRightDown) {
                        isDragging = false;
                    }
                    lastLeftDown = nowLeftDown;
                    lastRightDown = nowRightDown;
                    lastCursorX = nowCursorX;
                    lastCursorY = nowCursorY;

                    auto lastScrollDeltaY = getLastScrollDeltaY();
                    if (std::abs(lastScrollDeltaY) > 0.01) {
                        controller::getMainSceneCamera()->moveForwardBackward((float) lastScrollDeltaY);
                    }
                }
            }
            auto texture3dView = gui_internal::convertTextureId(config::getBool(config::DISPLAY_SELECTION_BUFFER)
                                                ? controller::getMainScene()->getSelectionImage()->getTexBO()
                                                : controller::getMainScene()->getImage().getTexBO());
            ImGui::ImageButton(texture3dView, regionAvail, ImVec2(0, 1), ImVec2(1, 0), 0);
            ImGui::EndChild();
        }
        ImGui::End();
    }
}