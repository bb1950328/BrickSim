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

        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const glm::mat4& absTransformation, const std::shared_ptr<Connector>& connector);
        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const glm::mat4& absTransformation, const std::shared_ptr<ClipConnector>& connector);
        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const glm::mat4& absTransformation, const std::shared_ptr<CylindricalConnector>& connector);
        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const glm::mat4& absTransformation, const std::shared_ptr<FingerConnector>& connector);
        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const glm::mat4& absTransformation, const std::shared_ptr<GenericConnector>& connector);

        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const std::shared_ptr<Connector>& connector);
        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const std::shared_ptr<ClipConnector>& connector);
        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const std::shared_ptr<CylindricalConnector>& connector);
        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const std::shared_ptr<FingerConnector>& connector);
        PairCheckData(const std::shared_ptr<etree::LdrNode>& node, const std::shared_ptr<GenericConnector>& connector);

        PairCheckData(const glm::mat4& absTransformation, const std::shared_ptr<Connector>& connector);
        PairCheckData(const glm::mat4& absTransformation, const std::shared_ptr<ClipConnector>& connector);
        PairCheckData(const glm::mat4& absTransformation, const std::shared_ptr<CylindricalConnector>& connector);
        PairCheckData(const glm::mat4& absTransformation, const std::shared_ptr<FingerConnector>& connector);
        PairCheckData(const glm::mat4& absTransformation, const std::shared_ptr<GenericConnector>& connector);
    };

    /**
     * this class checks if two specific connectors of two specific parts are connected
     */
    class PairChecker {
        const float absoluteDirectionAngleDifference;
        const bool sameDir;
        const bool oppositeDir;

        bool findGenericGeneric();
        bool findCylCyl();
        bool findFingerFinger();
        bool findClipCyl();

        std::optional<float> projectConnectorsWithLength();

    protected:
        const PairCheckData a;
        const PairCheckData b;
        virtual void addConnection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom dof) = 0;

    public:
        PairChecker(PairCheckData a, PairCheckData b);
        void findConnections();
    };

    class ConnectionGraphPairChecker : public PairChecker {
        ConnectionGraph& result;

    protected:
        void addConnection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom dof) override;

    public:
        ConnectionGraphPairChecker(PairCheckData a, PairCheckData b, ConnectionGraph& result);
    };

    class VectorPairChecker : public PairChecker {
        std::vector<std::array<std::shared_ptr<Connector>, 2>>& result;

    protected:
        void addConnection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom dof) override;

    public:
        VectorPairChecker(const PairCheckData& a, const PairCheckData& b, std::vector<std::array<std::shared_ptr<Connector>, 2>>& result);
    };
}
