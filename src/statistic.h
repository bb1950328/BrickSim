// counters.h
// Created by bb1950328 on 05.10.20.
//

#ifndef BRICKSIM_STATISTIC_H
#define BRICKSIM_STATISTIC_H

namespace statistic {
    class Counters {
    public:
        static long totalBrickCount;
        static long individualBrickCount;
        static size_t vramUsageBytes;
    };

    void print();
}

#endif //BRICKSIM_STATISTIC_H
