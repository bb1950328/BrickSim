#include "metrics.h"

namespace metrics {
    long individualBrickCount = 0;
    size_t vramUsageBytes = 0;
    size_t thumbnailBufferUsageBytes = 0;
    float lastElementTreeRereadMs = 0;
    float lastThumbnailRenderingTimeMs = 0;
    std::vector<std::pair<std::string, float>> lastWindowDrawingTimesUs = {};
    float lastSceneRenderTimeMs;
    std::vector<std::pair<const char*, unsigned int>> mainloopTimePointsUs;
}
