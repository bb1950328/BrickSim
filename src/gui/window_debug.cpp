//
// Created by bb1950328 on 15.11.2020.
//

#include "gui.h"
#include "../controller.h"

namespace gui {
    void windows::drawDebugWindow(bool* show) {
        ImGui::Begin(ICON_FA_BUG" Debug", show);
        long lastFrameTime = controller::getLastFrameTime();
        ImGui::Text(ICON_FA_CHART_LINE" Application average %.3f ms/frame (%.1f FPS)", lastFrameTime / 1000.0, 1000000.0 / lastFrameTime);
        ImGui::Text(ICON_FA_MEMORY" Total graphics buffer size: %s", util::formatBytesValue(statistic::vramUsageBytes).c_str());
        ImGui::Text(ICON_FA_IMAGES" Total thumbnail buffer size: %s", util::formatBytesValue(statistic::thumbnailBufferUsageBytes).c_str());
        ImGui::Text(ICON_FA_SYNC" Last element tree reread: %.2f ms", statistic::lastElementTreeRereadMs);
        ImGui::Text(ICON_FA_HISTORY" Last thumbnail render time: %.2f ms", statistic::lastThumbnailRenderingTimeMs);
        const auto &bgTasks = controller::getBackgroundTasks();
        if (!bgTasks.empty()) {
            ImGui::Text("%llu background tasks:", bgTasks.size());
            for (const auto &task : bgTasks) {
                ImGui::BulletText("%s", task.second->getName().c_str());
            }
        }

        ImGui::BeginChild(ICON_FA_WINDOW_RESTORE" Window drawing times", ImVec2(0, ImGui::GetFontSize()*7), true);
        for (const auto &item : statistic::lastWindowDrawingTimesMs) {
            ImGui::Text("%s: %.3f ms", item.first.c_str(), item.second);
        }
        ImGui::EndChild();

        if (ImGui::Button(ICON_FA_SYNC" Reread element tree now")) {
            controller::setElementTreeChanged(true);
        }
        ImGui::End();
    }
}