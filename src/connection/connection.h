#pragma once
#include "connector/connector.h"
#include "glm/glm.hpp"
#include <vector>

namespace bricksim::connection {
    constexpr static float PARALLELITY_ANGLE_TOLERANCE = .018f;//around 1°
    constexpr static float PARALLELITY_ANGLE_TOLERANCE_SQUARED = PARALLELITY_ANGLE_TOLERANCE * PARALLELITY_ANGLE_TOLERANCE;
    constexpr static float COLINEARITY_TOLERANCE_LDU = .1f;
    constexpr static float POSITION_TOLERANCE_LDU = .1f;
    constexpr static float CONNECTION_RADIUS_TOLERANCE = 1.f;

    struct RotationPossibility {
        glm::vec3 origin;
        glm::vec3 axis;
        RotationPossibility(const glm::vec3& origin, const glm::vec3& axis);
        bool operator==(const RotationPossibility& rhs) const;
        bool operator!=(const RotationPossibility& rhs) const;
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
    };

    class Connection {
    public:
        std::shared_ptr<Connector> connectorA;
        std::shared_ptr<Connector> connectorB;
        DegreesOfFreedom degreesOfFreedom;

        Connection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB);
        Connection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom degreesOfFreedom);
        bool operator==(const Connection& rhs) const;
        bool operator!=(const Connection& rhs) const;
    };
}
