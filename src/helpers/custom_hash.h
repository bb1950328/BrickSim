#pragma once
#include "color.h"
#include <array>
#include <functional>

namespace std {
    template<typename T, std::size_t N>
    struct hash<std::array<T*, N>> {
        std::size_t operator()(const std::array<T*, N>& value) const noexcept {
            std::size_t result = 17;
            for (const auto& item: value) {
                result = result * 31 + hash<T*>()(item);
            }
            return result;
        }
    };

    template<>
    struct hash<pair<string, string>> {
        std::size_t operator()(const pair<string, string>& value) const noexcept {
            return hash<string>()(value.first) * 31 + hash<string>()(value.second);
        }
    };
}
