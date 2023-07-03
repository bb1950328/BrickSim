#pragma once
#include <array>
#include <functional>

//todo move other custom hash functions scattered throughout the codebase
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
}
