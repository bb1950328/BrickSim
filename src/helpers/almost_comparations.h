#pragma once
#include <cmath>

namespace bricksim {
    template<typename T>
    constexpr bool almostGreater(T a, T b, T epsilon = .001) {
        return a - b + epsilon > 0;
    }

    template<typename T>
    constexpr bool almostLess(T a, T b, T epsilon = .001) {
        return b - a + epsilon > 0;
    }

    template<typename T>
    constexpr bool almostEqual(T a, T b, T epsilon = .001) {
        return std::abs(a - b) < epsilon;
    }
}
