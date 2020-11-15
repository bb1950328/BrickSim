// counters.h
// Created by bb1950328 on 05.10.20.
//

#ifndef BRICKSIM_STATISTIC_H
#define BRICKSIM_STATISTIC_H

#include <vector>

namespace statistic {
    extern long totalBrickCount;
    extern long individualBrickCount;
    extern size_t vramUsageBytes;
    extern size_t thumbnailBufferUsageBytes;
    extern float lastElementTreeRereadMs;
    extern float lastThumbnailRenderingTimeMs;
    extern std::vector<std::pair<std::string, float>> lastWindowDrawingTimesMs;

    void print();
}

#endif //BRICKSIM_STATISTIC_H
