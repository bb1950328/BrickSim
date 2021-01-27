// counters.cpp
// Created by bb1950328 on 05.10.20.
//

#include <iostream>
#include <map>
#include <spdlog/spdlog.h>
#include "statistic.h"
#include "helpers/util.h"

namespace statistic {
    long individualBrickCount = 0;
    size_t vramUsageBytes = 0;
    size_t thumbnailBufferUsageBytes = 0;
    float lastElementTreeRereadMs = 0;
    float lastThumbnailRenderingTimeMs = 0;
    std::vector<std::pair<std::string, float>> lastWindowDrawingTimesMs = {};
}
