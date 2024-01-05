#pragma once

#include "connection.h"
#include "magic_enum.hpp"
#include "pair_checker.h"
#include <array>
#include <vector>

namespace bricksim::connection {
    namespace {
        struct DirectionSet : std::vector<glm::vec3> {
            DirectionSet();
            std::size_t getIndex(const glm::vec3& direction);
        };

        struct DirectionsContainer {
            DirectionSet gendered;
            DirectionSet ungendered;
        };

        struct BaseConnectorGrouping {
            glm::mat4 transformation = glm::mat4(1.f);
            DirectionsContainer& directions;
            std::array<std::vector<std::vector<std::shared_ptr<Connector>>>, 2> gendered;
            std::vector<std::vector<std::shared_ptr<Connector>>> ungendered;
            std::vector<std::shared_ptr<Connector>> undirected;

            explicit BaseConnectorGrouping(DirectionsContainer& directions);

            void addGendered(const std::shared_ptr<Connector>& connector, Gender gender);
            void addUngendered(const std::shared_ptr<Connector>& connector);
            void addUndirected(const std::shared_ptr<Connector>& connector);

            void addAll(const std::vector<std::shared_ptr<Connector>>& connectors);

            void updateContainers();
        };

        struct SingleConnectorGrouping : BaseConnectorGrouping {
            DirectionsContainer directions;
            SingleConnectorGrouping();
        };

        struct DoubleConnectorGrouping {
            DirectionsContainer directions;
            BaseConnectorGrouping a{directions};
            BaseConnectorGrouping b{directions};
        };
    }

    class ConnectionCheck {
    public:
        explicit ConnectionCheck(PairCheckResultConsumer& resultConsumer);
        void checkForConnected(const connector_container_t& connectors);
        void checkForConnected(const std::shared_ptr<etree::MeshNode>& nodeA, const std::shared_ptr<etree::MeshNode>& nodeB);
        void checkForConnected(const connector_container_t& connectorsA,
                               const connector_container_t& connectorsB,
                               const glm::mat4& transfA,
                               const glm::mat4& transfB);

    protected:
        void checkBruteForceAvsA(const connector_container_t& conns);

        void checkBruteForceAvsB(const connector_container_t& connsA,
                                 const connector_container_t& connsB,
                                 const glm::mat4& aTransf,
                                 const glm::mat4& bTransf);
        void checkDirectionalAdvancedAvsB(size_t directionIdx,
                                          const connector_container_t& aConns,
                                          const connector_container_t& bConns,
                                          const glm::mat4& aTransf,
                                          const glm::mat4& bTransf);
        void checkDirectionalAdvancedAvsA(size_t directionIdx, const connector_container_t& conns);
        void checkDirectionalAvsB(const DirectionSet& directions,
                                  const std::vector<connector_container_t>& aConns,
                                  const std::vector<connector_container_t>& bConns,
                                  const glm::mat4& aTransf,
                                  const glm::mat4& bTransf);
        void checkDirectionalAvsA(const DirectionSet& directions,
                                  const std::vector<connector_container_t>& connectors);

    private:
        PairCheckResultConsumer& result;
    };
}
