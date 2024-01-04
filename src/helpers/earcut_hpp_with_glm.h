#pragma once

//relative path is intentional. earcut.hpp is not included in CMakeLists.txt so all files have to use this file instead of including earcut.hpp directly
#include "../lib/earcut.hpp/include/mapbox/earcut.hpp"

#include <glm/glm.hpp>

namespace mapbox::util {
    template<>
    struct nth<0, glm::vec2> {
        inline static auto get(const glm::vec2& t) {
            return t.x;
        };
    };

    template<>
    struct nth<1, glm::vec2> {
        inline static auto get(const glm::vec2& t) {
            return t.y;
        };
    };
}
