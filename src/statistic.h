

#ifndef BRICKSIM_STATISTIC_H
#define BRICKSIM_STATISTIC_H

#include <vector>

namespace statistic {
    extern long individualBrickCount;
    extern size_t vramUsageBytes;
    extern size_t thumbnailBufferUsageBytes;
    extern float lastElementTreeRereadMs;
    extern float lastThumbnailRenderingTimeMs;
    extern std::vector<std::pair<std::string, float>> lastWindowDrawingTimesMs;
}

#endif //BRICKSIM_STATISTIC_H
