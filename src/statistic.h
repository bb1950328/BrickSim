// counters.h
// Created by bab21 on 05.10.20.
//

#ifndef BRICKSIM_STATISTIC_H
#define BRICKSIM_STATISTIC_H

namespace stats {
    class Counters {
    public:
        static long totalBrickCount;
        static long individualBrickCount;
    };

    void print();
}

#endif //BRICKSIM_STATISTIC_H
