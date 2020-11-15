// counters.cpp
// Created by bb1950328 on 05.10.20.
//

#include <iostream>
#include "statistic.h"
#include "helpers/util.h"

namespace statistic {
    long totalBrickCount = 0;
    long individualBrickCount = 0;
    size_t vramUsageBytes = 0;
    size_t thumbnailBufferUsageBytes = 0;
    float lastElementTreeRereadMs = 0;
    float lastThumbnailRenderingTimeMs = 0;

    void print() {
        std::cout << "===== Statistics =====" << std::endl;
        std::cout << "Brick Count: " << individualBrickCount << " different bricks, " << totalBrickCount << " total" << std::endl;
        std::cout << "VRAM Usage: " << util::formatBytesValue(vramUsageBytes) << std::endl;
    }
}
