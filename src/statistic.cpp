// counters.cpp
// Created by bb1950328 on 05.10.20.
//

#include <iostream>
#include "statistic.h"
#include "helpers/util.h"

namespace statistic {
    long Counters::totalBrickCount = 0;
    long Counters::individualBrickCount = 0;
    size_t Counters::vramUsageBytes = 0;
    size_t Counters::thumbnailBufferUsageBytes = 0;

    void print() {
        std::cout << "===== Statistics =====" << std::endl;
        std::cout << "Brick Count: " << Counters::individualBrickCount << " different bricks, "
                  << Counters::totalBrickCount << " total" << std::endl;
        std::cout << "VRAM Usage: " << util::formatBytesValue(Counters::vramUsageBytes) << std::endl;
    }
}
