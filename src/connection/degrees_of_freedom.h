#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace bricksim::connection {
    struct RotationPossibility {
        glm::vec3 origin;
        glm::vec3 axis;
        RotationPossibility(const glm::vec3& origin, const glm::vec3& axis);
        bool operator==(const RotationPossibility& rhs) const;
        bool operator!=(const RotationPossibility& rhs) const;
        /// returns true if colinear
        bool compatible(const RotationPossibility& other) const;
    };

    class DegreesOfFreedom {
    public:
        std::vector<glm::vec3> slideDirections;
        std::vector<RotationPossibility> rotationPossibilities;

        DegreesOfFreedom();
        DegreesOfFreedom(const std::vector<glm::vec3>& slideDirections,
                         const std::vector<RotationPossibility>& rotationPossibilities);
        bool operator==(const DegreesOfFreedom& rhs) const;
        bool operator!=(const DegreesOfFreedom& rhs) const;
        bool empty() const;

        static DegreesOfFreedom reduce(const std::vector<DegreesOfFreedom>& dofs);
    };
}
