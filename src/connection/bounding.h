#pragma once

#include <glm/vec3.hpp>
#include <variant>
namespace bricksim::connection {
    struct BoundingPnt {
        bool operator==(const BoundingPnt& rhs) const = default;
    };

    struct BoundingBox {
        glm::vec3 radius;

        bool operator==(const BoundingBox& rhs) const = default;
    };
    struct BoundingCube {
        float radius;

        bool operator==(const BoundingCube& rhs) const = default;
    };
    struct BoundingCyl {
        float radius;
        float length;

        bool operator==(const BoundingCyl& rhs) const = default;
    };
    struct BoundingSph {
        float radius;

        bool operator==(const BoundingSph& rhs) const = default;
    };

    using bounding_variant_t = std::variant<BoundingPnt, BoundingBox, BoundingCube, BoundingCyl, BoundingSph>;
}
