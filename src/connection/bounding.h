#pragma once

#include <variant>
namespace bricksim::connection {
    struct BoundingPnt {
        bool operator==(const BoundingPnt& rhs) const;
        bool operator!=(const BoundingPnt& rhs) const;
    };
    struct BoundingBox {
        float x;
        float y;
        float z;

        bool operator==(const BoundingBox& rhs) const;
        bool operator!=(const BoundingBox& rhs) const;
    };
    struct BoundingCube {
        float size;

        bool operator==(const BoundingCube& rhs) const;
        bool operator!=(const BoundingCube& rhs) const;
    };
    struct BoundingCyl {
        float radius;
        float length;

        bool operator==(const BoundingCyl& rhs) const;
        bool operator!=(const BoundingCyl& rhs) const;
    };
    struct BoundingSph {
        float radius;

        bool operator==(const BoundingSph& rhs) const;
        bool operator!=(const BoundingSph& rhs) const;
    };

    typedef std::variant<BoundingPnt, BoundingBox, BoundingCube, BoundingCyl, BoundingSph> bounding_variant_t;
}
