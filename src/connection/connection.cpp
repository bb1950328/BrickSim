#include "connection.h"
#include "../helpers/geometry.h"
#include "magic_enum.hpp"
#include <spdlog/fmt/fmt.h>

#include <utility>

namespace bricksim::connection {

    DegreesOfFreedom::DegreesOfFreedom(const std::vector<glm::vec3>& slideDirections,
                                       const std::vector<RotationPossibility>& rotationPossibilities) :
        slideDirections(slideDirections),
        rotationPossibilities(rotationPossibilities) {
    }
    bool DegreesOfFreedom::operator==(const DegreesOfFreedom& rhs) const {
        return util::vecVecEpsilonEqual(slideDirections, rhs.slideDirections, .01f)
               && rotationPossibilities == rhs.rotationPossibilities;
    }
    bool DegreesOfFreedom::operator!=(const DegreesOfFreedom& rhs) const {
        return !(rhs == *this);
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

    Connection::Connection(const std::shared_ptr<Connector>& connectorA,
                           const std::shared_ptr<Connector>& connectorB,
                           DegreesOfFreedom degreesOfFreedom) :
        connectorA(connectorA),
        connectorB(connectorB),
        degreesOfFreedom(std::move(degreesOfFreedom)) {
    }
    Connection::Connection(const std::shared_ptr<Connector>& connectorA,
                           const std::shared_ptr<Connector>& connectorB) :
        connectorA(connectorA),
        connectorB(connectorB),
        degreesOfFreedom() {
    }
    bool Connection::operator==(const Connection& rhs) const {
        return connectorA == rhs.connectorA && connectorB == rhs.connectorB && degreesOfFreedom == rhs.degreesOfFreedom;
    }
    bool Connection::operator!=(const Connection& rhs) const {
        return !(rhs == *this);
    }
}
