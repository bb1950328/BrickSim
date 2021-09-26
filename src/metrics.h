#pragma once

#include <string>
#include <vector>

#ifndef NDEBUG
    #include <mutex>
#endif

namespace bricksim::metrics {
    extern long individualBrickCount;
    extern size_t vramUsageBytes;
    extern size_t thumbnailBufferUsageBytes;
    extern float lastElementTreeRereadMs;
    extern float lastThumbnailRenderingTimeMs;
    extern std::vector<std::pair<std::string, float>> lastWindowDrawingTimesUs;
    extern float lastSceneRenderTimeMs;
    extern size_t memorySavedByDeletingVertexData;
#ifndef NDEBUG
    extern std::mutex ldrFileElementInstanceCountMtx;
    extern size_t ldrFileElementInstanceCount;
#endif
}
