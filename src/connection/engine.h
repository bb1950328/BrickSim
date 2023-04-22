#pragma once

#include "../graphics/mesh/mesh_collection.h"
#include "data.h"
namespace bricksim::connection::engine {
    namespace {
        constexpr float PARALLELITY_ANGLE_TOLERANCE = .001f;
        constexpr float COLINEARITY_TOLERANCE_LDU = .1f;
        constexpr float POSITION_TOLERANCE_LDU = .1f;
        constexpr float CONNECTION_RADIUS_TOLERANCE = 1.f;

        struct ConnectorCheckData {
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

            ConnectorCheckData(const std::shared_ptr<etree::LdrNode>& node, const std::shared_ptr<Connector>& connector);
        };

        class ConnectionChecker {
            const ConnectorCheckData a;
            const ConnectorCheckData b;
            std::vector<std::shared_ptr<Connection>>& result;
            const float absoluteDirectionAngleDifference;
            const bool sameDir;
            const bool oppositeDir;

            bool findGenericGeneric();
            bool findCylCyl();
            bool findFingerFinger();
            bool findClipCyl();

            std::optional<float> projectConnectorsWithLength();

        public:
            ConnectionChecker(ConnectorCheckData a, ConnectorCheckData b, std::vector<std::shared_ptr<Connection>>& result);
            void findConnections();
        };
    }
    std::vector<std::shared_ptr<Connection>> findConnections(const std::shared_ptr<etree::LdrNode>& a, const std::shared_ptr<etree::LdrNode>& b);
    ConnectionGraph findConnections(const std::shared_ptr<etree::Node>& node, const mesh::SceneMeshCollection& meshCollection);
    ConnectionGraph findConnections(const std::shared_ptr<etree::LdrNode>& activeNode, const std::shared_ptr<etree::Node>& passiveNode, const mesh::SceneMeshCollection& meshCollection);
    void findConnections(const std::shared_ptr<etree::LdrNode>& activeNode, const std::shared_ptr<etree::Node>& passiveNode, const mesh::SceneMeshCollection& meshCollection, ConnectionGraph& result);
}
