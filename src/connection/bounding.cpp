#include "bounding.h"

namespace bricksim::connection {
    bool BoundingPnt::operator==(const BoundingPnt& rhs) const {
        return true;
    }
    bool BoundingPnt::operator!=(const BoundingPnt& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingBox::operator==(const BoundingBox& rhs) const {
        return x == rhs.x
               && y == rhs.y
               && z == rhs.z;
    }
    bool BoundingBox::operator!=(const BoundingBox& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingCube::operator==(const BoundingCube& rhs) const {
        return size == rhs.size;
    }
    bool BoundingCube::operator!=(const BoundingCube& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingCyl::operator==(const BoundingCyl& rhs) const {
        return radius == rhs.radius
               && length == rhs.length;
    }
    bool BoundingCyl::operator!=(const BoundingCyl& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingSph::operator==(const BoundingSph& rhs) const {
        return radius == rhs.radius;
    }
    bool BoundingSph::operator!=(const BoundingSph& rhs) const {
        return !(rhs == *this);
    }
}