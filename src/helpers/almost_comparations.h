#pragma once
#include "glm/glm.hpp"
#include <cmath>

namespace bricksim {
    template<typename T>
        requires std::is_arithmetic_v<T>
    constexpr bool almostGreater(T a, T b, T epsilon = .001) {
        return a - b + epsilon > 0;
    }

    template<typename T>
        requires std::is_arithmetic_v<T>
    constexpr bool almostLess(T a, T b, T epsilon = .001) {
        return b - a + epsilon > 0;
    }

    template<typename T, typename E>
        requires std::is_arithmetic_v<T>
    constexpr bool almostEqual(T a, T b, E epsilon = .001) {
        return std::abs(a - b) < epsilon;
    }

    template<typename T>
        requires std::is_arithmetic_v<T>
    constexpr bool almostEqual(T a, T b, float epsilon = .001) {
        return std::abs(a - b) < epsilon;
    }

    template<typename E, glm::length_t C, glm::length_t R>
    constexpr bool almostEqual(const glm::mat<C, R, E>& a, const glm::mat<C, R, E>& b, E epsilon = .001) {
        for (int ic = 0; ic < C; ++ic) {
            for (int ir = 0; ir < R; ++ir) {
                if (!almostEqual(a[ic][ir], b[ic][ir], epsilon)) {
                    return false;
                }
            }
        }
        return true;
    }
}
