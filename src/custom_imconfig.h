#pragma once

#include "helpers/color.h"
#include <glm/glm.hpp>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#define IM_VEC2_CLASS_EXTRA      \
    ImVec2(const glm::vec2& f) { \
        x = f.x;                 \
        y = f.y;                 \
    }                            \
    operator glm::vec2() const { return glm::vec2(x, y); }

#define IM_VEC4_CLASS_EXTRA                                      \
    ImVec4(const glm::vec4& f) {                                 \
        x = f.x;                                                 \
        y = f.y;                                                 \
        z = f.z;                                                 \
        w = f.w;                                                 \
    }                                                            \
    operator glm::vec4() const { return glm::vec4(x, y, z, w); } \
    ImVec4(const bricksim::color::RGB& c) {                      \
        const auto f = c.asGlmVector();                          \
        x = f.x;                                                 \
        y = f.y;                                                 \
        z = f.z;                                                 \
        w = 1.0f;                                                \
    }
