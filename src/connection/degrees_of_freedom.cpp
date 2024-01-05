#include "degrees_of_freedom.h"
#include "../helpers/util.h"
#include "connection.h"

namespace bricksim::connection {
    DegreesOfFreedom::DegreesOfFreedom(const std::vector<glm::vec3>& slideDirections,
                                       const std::vector<RotationPossibility>& rotationPossibilities) :
        slideDirections(slideDirections),
        rotationPossibilities(rotationPossibilities) {}

    bool DegreesOfFreedom::operator==(const DegreesOfFreedom& rhs) const {
        return util::vecVecEpsilonEqual(slideDirections, rhs.slideDirections, .01f)
               && rotationPossibilities == rhs.rotationPossibilities;
    }

    bool DegreesOfFreedom::operator!=(const DegreesOfFreedom& rhs) const {
        return !(rhs == *this);
    }

    DegreesOfFreedom DegreesOfFreedom::reduce(const std::vector<DegreesOfFreedom>& dofs) {
        if (dofs.empty()) {
            return {};
        }
        const auto& first = dofs[0];
        if (dofs.size() == 1) {
            return first;
        }
        DegreesOfFreedom result = first;
        bool skip = true;
        for (const auto& item: dofs) {
            if (skip) {
                skip = false;
                continue;
            }
            std::remove_if(result.slideDirections.begin(), result.slideDirections.end(), [&item, &result](const auto& dir) {
                const auto it = std::find_if(item.slideDirections.cbegin(), item.slideDirections.cend(), [&dir](const auto& d) {
                    return glm::length2(glm::cross(d, dir)) < PARALLELITY_ANGLE_TOLERANCE_SQUARED;
                });
                return it == result.slideDirections.end();
            });

            std::remove_if(result.rotationPossibilities.begin(), result.rotationPossibilities.end(), [&item](const RotationPossibility& possibility) {
                const auto it = std::find_if(item.rotationPossibilities.begin(), item.rotationPossibilities.end(), [&possibility](const RotationPossibility& po) {
                    return possibility.compatible(po);
                });
                return it == item.rotationPossibilities.end();
            });
        }
        return DegreesOfFreedom();
    }

    bool DegreesOfFreedom::empty() const {
        return slideDirections.empty() && rotationPossibilities.empty();
    }

    DegreesOfFreedom::DegreesOfFreedom() = default;

    RotationPossibility::RotationPossibility(const glm::vec3& origin, const glm::vec3& axis) :
        origin(origin), axis(axis) {}

    bool RotationPossibility::operator==(const RotationPossibility& rhs) const {
        return glm::all(glm::epsilonEqual(origin, rhs.origin, .1f))
               && glm::all(glm::epsilonEqual(axis, rhs.axis, .01f));
    }

    bool RotationPossibility::operator!=(const RotationPossibility& rhs) const {
        return !(rhs == *this);
    }

    bool RotationPossibility::compatible(const RotationPossibility& other) const {
        if (glm::length2(glm::cross(other.axis, axis)) < PARALLELITY_ANGLE_TOLERANCE_SQUARED) {
            const auto startDiff = other.origin - origin;
            const auto projectionLength = glm::dot(startDiff, axis);
            const auto distancePointToLine = glm::length(glm::cross(startDiff, axis));
            return distancePointToLine <= COLINEARITY_TOLERANCE_LDU;
        }
        return false;
    }
}
