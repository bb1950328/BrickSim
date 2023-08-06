#pragma once

#include "data.h"
namespace bricksim::connection {

    struct PairCheckData {
        glm::mat4 absTransformation;
        glm::vec3 absStart;
        glm::vec3 absEnd;//=absStart for connectors that do not have a length
        glm::vec3 absDirection;
        const std::shared_ptr<Connector>& connector;
        ClipConnector* clip;
        CylindricalConnector* cyl;
        FingerConnector* finger;
        GenericConnector* generic;

        PairCheckData(const glm::mat4& absTransformation,
                      const std::shared_ptr<Connector>& connector,
                      const glm::vec3 absStart,
                      const glm::vec3 absDirection);
        PairCheckData(const glm::mat4& absTransformation,
                      const std::shared_ptr<Connector>& connector);

        PairCheckData(const std::shared_ptr<etree::MeshNode>& node,
                      const std::shared_ptr<Connector>& connector);
    };

    class PairCheckResultConsumer {
    public:
        virtual void addConnection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom dof) = 0;
    };

    /**
     * this class checks if two specific connectors of two specific parts are connected
     */
    class PairChecker {
        const bool sameDir;
        const bool oppositeDir;
        PairCheckResultConsumer& resultConsumer;

        void findGenericGeneric();
        void findCylCyl();
        void findFingerFinger();
        void findClipCyl(const PairCheckData& clipData, const PairCheckData& cylData);

        std::optional<float> projectConnectorsWithLength(float aLength, float bLength);

    protected:
        const PairCheckData& a;
        const PairCheckData& b;
        void addConnection(DegreesOfFreedom dof);

    public:
        PairChecker(const PairCheckData& a, const PairCheckData& b, PairCheckResultConsumer& resultConsumer);
        void findConnections();
    };

    class ConnectionGraphPairCheckResultConsumer : public PairCheckResultConsumer {
        const ConnectionGraph::node_t& nodeA;
        const ConnectionGraph::node_t& nodeB;
        ConnectionGraph& result;

    protected:
        void addConnection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom dof) override;

    public:
        ConnectionGraphPairCheckResultConsumer(const ConnectionGraph::node_t& nodeA, const ConnectionGraph::node_t& nodeB, ConnectionGraph& result);
    };

    class VectorPairCheckResultConsumer : public PairCheckResultConsumer {
        std::vector<std::array<std::shared_ptr<Connector>, 2>> result;

    protected:
        void addConnection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom dof) override;

    public:
        const std::vector<std::array<std::shared_ptr<Connector>, 2>>& getResult() const;
    };
}
