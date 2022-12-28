#pragma once

#include <glm/fwd.hpp>
#include <map>
#include <set>
#include <robin_hood.h>

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
    using uomap_t = robin_hood::unordered_map<K, V>;

    template<typename T>
    using oset_t = std::set<T>;

    template<typename T>
    using uoset_t = robin_hood::unordered_set<T>;
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
