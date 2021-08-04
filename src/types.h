#pragma once

#include <glm/fwd.hpp>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace bricksim {
    typedef unsigned char layer_t;
    typedef unsigned char scene_id_t;
    typedef unsigned int element_id_t;
    typedef uint8_t color_component_t;
    typedef unsigned int texture_id_t;
    typedef uint64_t mesh_identifier_t;

    template <typename K, typename V>
    using omap_t = std::map<K, V>;
    template <typename K, typename V>
    using uomap_t = std::unordered_map<K, V>;

    template <typename T>
    using oset_t = std::set<T>;

    template <typename T>
    using uoset_t = std::unordered_set<T>;
}
namespace glm {
    typedef vec<1, unsigned short, defaultp> usvec1;
    typedef vec<2, unsigned short, defaultp> usvec2;
    typedef vec<3, unsigned short, defaultp> usvec3;
    typedef vec<4, unsigned short, defaultp> usvec4;
    typedef vec<1, signed short, defaultp> svec1;
    typedef vec<2, signed short, defaultp> svec2;
    typedef vec<3, signed short, defaultp> svec3;
    typedef vec<4, signed short, defaultp> svec4;
}
