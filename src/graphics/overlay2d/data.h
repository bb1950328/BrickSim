#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace bricksim::overlay2d {
    typedef glm::vec2 coord_t;

    typedef float length_t;

    class Vertex {
    public:
        glm::vec2 position;
        glm::vec3 color;
        Vertex(const glm::vec2& position, const glm::vec3& color);
        Vertex() = default;
    };

    struct VertexRange {
        unsigned int start;
        unsigned int count;
    };
}
