

#include "gui.h"
#include "../controller.h"

namespace gui {
    void windows::drawDebugWindow(bool* show) {
        ImGui::Begin(WINDOW_NAME_DEBUG, show);
        const auto lastFrameTimes = controller::getLastFrameTimes();
        const auto count = std::get<0>(lastFrameTimes);
        const auto arrPtr = std::get<1>(lastFrameTimes);
        const auto startIdx = std::get<2>(lastFrameTimes);
        const auto endIdx = (startIdx - 1) % count;
        ImGui::Text(ICON_FA_CHART_LINE" Application render average %.3f ms/frame (%.1f FPS)", arrPtr[endIdx], 1000.0 / arrPtr[endIdx]);
        ImGui::PlotLines("ms/frame", arrPtr, count, startIdx);
        ImGui::Text(ICON_FA_STOPWATCH" Last 3D View render time: %.3f ms", metrics::last3DViewRenderTimeMs);
        ImGui::Text(ICON_FA_MEMORY" Total graphics buffer size: %s", util::formatBytesValue(metrics::vramUsageBytes).c_str());
        ImGui::Text(ICON_FA_IMAGES" Total thumbnail buffer size: %s", util::formatBytesValue(metrics::thumbnailBufferUsageBytes).c_str());
        ImGui::Text(ICON_FA_SYNC" Last element tree reread: %.2f ms", metrics::lastElementTreeRereadMs);
        ImGui::Text(ICON_FA_HISTORY" Last thumbnail render time: %.2f ms", metrics::lastThumbnailRenderingTimeMs);
        const auto &bgTasks = controller::getBackgroundTasks();
        if (!bgTasks.empty()) {
            ImGui::Text("%lu background tasks:", bgTasks.size());
            for (const auto &task : bgTasks) {
                ImGui::BulletText("%s", task.second->getName().c_str());
            }
        }

        for (const auto &timePointsUs : metrics::mainloopTimePointsUs) {
            ImGui::BulletText("%s: %u µs", timePointsUs.first, timePointsUs.second);
        }

        ImGui::BeginChild(ICON_FA_WINDOW_RESTORE" Window drawing times", ImVec2(0, ImGui::GetFontSize()*7), true);
        for (const auto &item : metrics::lastWindowDrawingTimesUs) {
            ImGui::Text("%s: %.1f µs", item.first.c_str(), item.second);
        }
        ImGui::EndChild();

        if (ImGui::Button(ICON_FA_SYNC" Reread element tree now")) {
            controller::setElementTreeChanged(true);
        }
        ImGui::End();
    }
}