#pragma once

#include "data.h"
namespace bricksim::connection {

    struct PairCheckData {
        std::shared_ptr<etree::LdrNode> node;
        glm::mat4 absTransformation;
        glm::vec3 absStart;
        glm::vec3 absEnd;//=absStart for connectors that do not have a length
        glm::vec3 absDirection;
        std::shared_ptr<Connector> connector;
        std::shared_ptr<ConnectorWithLength> connectorWithLength;
        std::shared_ptr<ClipConnector> clip;
        std::shared_ptr<CylindricalConnector> cyl;
        std::shared_ptr<FingerConnector> finger;
        std::shared_ptr<GenericConnector> generic;

        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const std::shared_ptr<Connector>& connector);
    };

    /**
     * this class checks if two specific connectors of two specific parts are connected
     */
    class PairChecker {
        constexpr static float PARALLELITY_ANGLE_TOLERANCE = .001f;
        constexpr static float COLINEARITY_TOLERANCE_LDU = .1f;
        constexpr static float POSITION_TOLERANCE_LDU = .1f;
        constexpr static float CONNECTION_RADIUS_TOLERANCE = 1.f;

        const PairCheckData a;
        const PairCheckData b;
        ConnectionGraph& result;
        const float absoluteDirectionAngleDifference;
        const bool sameDir;
        const bool oppositeDir;

        bool findGenericGeneric();
        bool findCylCyl();
        bool findFingerFinger();
        bool findClipCyl();

        std::optional<float> projectConnectorsWithLength();

    public:
        PairChecker(PairCheckData a, PairCheckData b, ConnectionGraph& result);
        void findConnections();
    };
}
