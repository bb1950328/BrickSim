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
        void checkForConnected(const std::vector<std::shared_ptr<Connector>>& connectors);
        void checkForConnected(const std::shared_ptr<etree::MeshNode>& nodeA, const std::shared_ptr<etree::MeshNode>& nodeB);

    protected:
        void checkBruteForceAvsA(const std::vector<std::shared_ptr<Connector>>& conns);

        void checkBruteForceAvsB(const std::vector<std::shared_ptr<Connector>>& connsA,
                                 const std::vector<std::shared_ptr<Connector>>& connsB,
                                 const glm::mat4& aTransf,
                                 const glm::mat4& bTransf);
        void checkDirectionalAdvancedAvsB(size_t directionIdx,
                                          const std::vector<std::shared_ptr<Connector>>& aConns,
                                          const std::vector<std::shared_ptr<Connector>>& bConns,
                                          const glm::mat4& aTransf,
                                          const glm::mat4& bTransf);
        void checkDirectionalAdvancedAvsA(size_t directionIdx, const std::vector<std::shared_ptr<Connector>>& conns);
        void checkDirectionalAvsB(const DirectionSet& directions,
                                  const std::vector<std::vector<std::shared_ptr<Connector>>>& aConns,
                                  const std::vector<std::vector<std::shared_ptr<Connector>>>& bConns,
                                  const glm::mat4& aTransf,
                                  const glm::mat4& bTransf);
        void checkDirectionalAvsA(const DirectionSet& directions,
                                  const std::vector<std::vector<std::shared_ptr<Connector>>>& connectors);

    private:
        PairCheckResultConsumer& result;
    };
}
