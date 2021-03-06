#pragma once

#include <string>
#include <vector>

namespace bricksim::metrics {
    extern long individualBrickCount;
    extern size_t vramUsageBytes;
    extern size_t thumbnailBufferUsageBytes;
    extern float lastElementTreeRereadMs;
    extern float lastThumbnailRenderingTimeMs;
    extern std::vector<std::pair<std::string, float>> lastWindowDrawingTimesUs;
    extern float lastSceneRenderTimeMs;
    extern std::vector<std::pair<const char*, unsigned int>> mainloopTimePointsUs;
}
