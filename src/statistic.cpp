// counters.cpp
// Created by bab21 on 05.10.20.
//

#include <iostream>
#include "statistic.h"

namespace stats {
    long Counters::totalBrickCount = 0;
    long Counters::individualBrickCount = 0;

    void print() {
        std::cout << "===== Statistics =====" << std::endl;
        std::cout << "Brick Count: " << Counters::individualBrickCount << " different bricks, "
                  << Counters::totalBrickCount << " total" << std::endl;
    }
}
