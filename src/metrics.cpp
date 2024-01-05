#include "metrics.h"

namespace bricksim::metrics {
    long individualBrickCount = 0;
    size_t vramUsageBytes = 0;
    size_t thumbnailBufferUsageBytes = 0;
    float lastElementTreeRereadMs = 0;
    float lastThumbnailRenderingTimeMs = 0;
    std::vector<std::pair<std::string, float>> lastWindowDrawingTimesUs = {};
    float lastSceneRenderTimeMs;
    size_t memorySavedByDeletingVertexData = 0;
    #ifndef NDEBUG
    //std::mutex ldrFileElementInstanceCountMtx;
    size_t ldrFileElementInstanceCount = 0;
    #endif
}
