#include "connection.h"


namespace bricksim::connection {

    Connection::Connection(const std::shared_ptr<Connector>& connectorA,
                           const std::shared_ptr<Connector>& connectorB) :
        connectorA(connectorA),
        connectorB(connectorB),
        degreesOfFreedom(),
        completelyUsedConnector({true, true}) {
    }
    Connection::Connection(const std::shared_ptr<Connector>& connectorA,
                           const std::shared_ptr<Connector>& connectorB,
                           DegreesOfFreedom degreesOfFreedom,
                           const std::array<bool, 2>& completelyUsedConnector) :
        connectorA(connectorA),
        connectorB(connectorB),
        degreesOfFreedom(std::move(degreesOfFreedom)),
        completelyUsedConnector(completelyUsedConnector) {
    }
    bool Connection::operator==(const Connection& rhs) const {
        return connectorA == rhs.connectorA
               && connectorB == rhs.connectorB
               && degreesOfFreedom == rhs.degreesOfFreedom
               && completelyUsedConnector == rhs.completelyUsedConnector;
    }
    bool Connection::operator!=(const Connection& rhs) const {
        return !(rhs == *this);
    }
}
