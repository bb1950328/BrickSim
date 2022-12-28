#pragma once

#include <variant>
namespace bricksim::connection {
    struct BoundingPnt {
        bool operator==(const BoundingPnt& rhs) const;
    };
    struct BoundingBox {
        float x;
        float y;
        float z;

        bool operator==(const BoundingBox& rhs) const;
    };
    struct BoundingCube {
        float size;

        bool operator==(const BoundingCube& rhs) const;
    };
    struct BoundingCyl {
        float radius;
        float length;

        bool operator==(const BoundingCyl& rhs) const;
    };
    struct BoundingSph {
        float radius;

        bool operator==(const BoundingSph& rhs) const;
    };

    using bounding_variant_t = std::variant<BoundingPnt, BoundingBox, BoundingCube, BoundingCyl, BoundingSph>;
}
