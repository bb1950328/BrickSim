#include "bounding.h"

namespace bricksim::connection {
    bool BoundingPnt::operator==(const BoundingPnt& rhs) const = default;
    bool BoundingBox::operator==(const BoundingBox& rhs) const = default;
    bool BoundingCube::operator==(const BoundingCube& rhs) const = default;
    bool BoundingCyl::operator==(const BoundingCyl& rhs) const = default;
    bool BoundingSph::operator==(const BoundingSph& rhs) const = default;
}
