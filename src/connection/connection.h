#pragma once
#include "connector/connector.h"
#include "degrees_of_freedom.h"
#include <array>

namespace bricksim::connection {
    constexpr static float PARALLELITY_ANGLE_TOLERANCE = .018f;//around 1°
    constexpr static float PARALLELITY_ANGLE_TOLERANCE_SQUARED = PARALLELITY_ANGLE_TOLERANCE * PARALLELITY_ANGLE_TOLERANCE;
    constexpr static float COLINEARITY_TOLERANCE_LDU = .1f;
    constexpr static float POSITION_TOLERANCE_LDU = .1f;
    constexpr static float CONNECTION_RADIUS_TOLERANCE = 1.f;

    class Connection {
    public:
        std::shared_ptr<Connector> connectorA;
        std::shared_ptr<Connector> connectorB;
        DegreesOfFreedom degreesOfFreedom;
        std::array<bool, 2> completelyUsedConnector;

        Connection(const std::shared_ptr<Connector>& connectorA,
                   const std::shared_ptr<Connector>& connectorB);
        Connection(const std::shared_ptr<Connector>& connectorA,
                   const std::shared_ptr<Connector>& connectorB,
                   DegreesOfFreedom degreesOfFreedom,
                   const std::array<bool, 2>& completelyUsedConnector);
        bool operator==(const Connection& rhs) const;
        bool operator!=(const Connection& rhs) const;
    };
}
