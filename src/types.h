#pragma once
#include "helpers/platform_detection.h"
#ifdef BRICKSIM_PLATFORM_MACOS
    #include <filesystem>
#endif
#include <ankerl/unordered_dense.h>
#include <glm/fwd.hpp>
#include <map>
#include <set>

namespace bricksim {
    using layer_t = unsigned char;
    using scene_id_t = unsigned char;
    using element_id_t = unsigned int;
    using color_component_t = uint8_t;
    using texture_id_t = unsigned int;
    using mesh_identifier_t = uint64_t;

    template<typename K, typename V>
    using omap_t = std::map<K, V>;
    template<typename K, typename V>
    using uomap_t = ankerl::unordered_dense::map<K, V>;

    template<typename T>
    using oset_t = std::set<T>;

    template<typename T>
    using uoset_t = ankerl::unordered_dense::set<T>;

    template<typename T>
    using hash = ankerl::unordered_dense::hash<T>;
}
namespace glm {
    using usvec1 = vec<1, unsigned short, defaultp>;
    using usvec2 = vec<2, unsigned short, defaultp>;
    using usvec3 = vec<3, unsigned short, defaultp>;
    using usvec4 = vec<4, unsigned short, defaultp>;
    using svec1 = vec<1, short, defaultp>;
    using svec2 = vec<2, short, defaultp>;
    using svec3 = vec<3, short, defaultp>;
    using svec4 = vec<4, short, defaultp>;
}
namespace std {
#ifdef BRICKSIM_PLATFORM_MACOS
    template<>
    struct hash<filesystem::path> {
        size_t operator()(const filesystem::path& value) const noexcept {
            return filesystem::hash_value(value);
        }
    };
#endif
    template<typename T>
    struct hash<vector<T>> {
        size_t operator()(const vector<T>& value) const noexcept {
            //https://stackoverflow.com/a/72073933/8733066
            std::size_t seed = value.size();
            for (const auto& v: value) {
                auto x = hash<T>()(v);
                x = ((x >> 16) ^ x) * 0x45d9f3b;
                x = ((x >> 16) ^ x) * 0x45d9f3b;
                x = (x >> 16) ^ x;
                seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}
