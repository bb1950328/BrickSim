//
// Created by Bader on 15.11.2020.
//

#include "gui.h"
#include "../controller.h"

namespace gui {
    void windows::drawDebugWindow(bool* show) {
        ImGui::Begin("Debug Information", show);
        long lastFrameTime = controller::getLastFrameTime();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", lastFrameTime / 1000.0, 1000000.0 / lastFrameTime);
        ImGui::Text("Total graphics buffer size: %s", util::formatBytesValue(statistic::vramUsageBytes).c_str());
        ImGui::Text("Total thumbnail buffer size: %s", util::formatBytesValue(statistic::thumbnailBufferUsageBytes).c_str());
        ImGui::Text("Last element tree reread: %.2f ms", statistic::lastElementTreeRereadMs);
        ImGui::Text("Last thumbnail render time: %.2f ms", statistic::lastThumbnailRenderingTimeMs);
        const auto &bgTasks = controller::getBackgroundTasks();
        if (!bgTasks.empty()) {
            ImGui::Text("%llu background tasks:", bgTasks.size());
            for (const auto &task : bgTasks) {
                ImGui::BulletText("%s", task.second->getTaskName().c_str());
            }
        }
        if (ImGui::Button("Reread element tree now")) {
            controller::setElementTreeChanged(true);
        }
        ImGui::End();
    }
}