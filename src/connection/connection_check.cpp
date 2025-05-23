#include "connection_check.h"
#include "../helpers/geometry.h"
#include "connector_data_provider.h"
#include <magic_enum/magic_enum.hpp>
#include "pair_checker.h"
#include <spdlog/spdlog.h>

namespace std {
    template<>
    struct hash<std::pair<int64_t, int64_t>> {
        std::size_t operator()(const std::pair<int64_t, int64_t>& value) const {
            return bricksim::util::combinedHash(value.first, value.second);
        }
    };
}

namespace bricksim::connection {
    namespace {
        constexpr auto M_IDX = *magic_enum::enum_index(Gender::M);
        constexpr auto F_IDX = *magic_enum::enum_index(Gender::F);

        SingleConnectorGrouping::SingleConnectorGrouping() :
            BaseConnectorGrouping(directions) {}

        BaseConnectorGrouping::BaseConnectorGrouping(DirectionsContainer& directions) :
            directions(directions) {}

        void BaseConnectorGrouping::addGendered(const std::shared_ptr<Connector>& connector, Gender gender) {
            const auto absDir = transformation * glm::vec4(connector->direction, 0.f);
            const auto dirIdx = directions.gendered.getIndex(absDir);
            updateContainers();
            auto& v = gendered[*magic_enum::enum_index(gender)];
            v[dirIdx].push_back(connector);
        }

        void BaseConnectorGrouping::addUngendered(const std::shared_ptr<Connector>& connector) {
            const auto absDir = transformation * glm::vec4(connector->direction, 0.f);
            const auto dirIdx = directions.ungendered.getIndex(absDir);
            updateContainers();
            ungendered[dirIdx].push_back(connector);
        }

        void BaseConnectorGrouping::addUndirected(const std::shared_ptr<Connector>& connector) {
            undirected.push_back(connector);
        }

        void BaseConnectorGrouping::addAll(const connector_container_t& connectors) {
            for (const auto& conn: connectors) {
                auto gender = Gender::F;
                switch (conn->type) {
                    case Connector::Type::CYLINDRICAL:
                        gender = std::dynamic_pointer_cast<CylindricalConnector>(conn)->gender;
                    case Connector::Type::CLIP:
                        addGendered(conn, gender);
                        break;
                    case Connector::Type::FINGER:
                        addUngendered(conn);
                        break;
                    case Connector::Type::GENERIC:
                        addUndirected(conn);
                        break;
                    default:
                        break;
                }
            }
        }

        void bricksim::connection::BaseConnectorGrouping::updateContainers() {
            gendered[M_IDX].resize(directions.gendered.size(), {});
            gendered[F_IDX].resize(directions.gendered.size(), {});
            ungendered.resize(directions.ungendered.size(), {});
        }

        DirectionSet::DirectionSet() :
            std::vector<glm::vec3>({
                    glm::vec3(1.f, 0.f, 0.f),
                    glm::vec3(0.f, 1.f, 0.f),
                    glm::vec3(0.f, 0.f, 1.f),
            }) {}

        std::size_t DirectionSet::getIndex(const glm::vec3& direction) {
            for (std::size_t i = 0; i < size(); ++i) {
                if (geometry::isAlmostParallel(at(i), direction)) {
                    return i;
                }
            }
            push_back(direction);
            return size() - 1;
        }
    }

    ConnectionCheck::ConnectionCheck(PairCheckResultConsumer& resultConsumer) :
        result(resultConsumer) {}

    void ConnectionCheck::checkForConnected(const connector_container_t& connectors) {
        SingleConnectorGrouping grouping;
        grouping.addAll(connectors);

        checkDirectionalAvsB(grouping.directions.gendered, grouping.gendered[M_IDX], grouping.gendered[F_IDX], glm::mat4(1.f), glm::mat4(1.f));
        checkDirectionalAvsA(grouping.directions.ungendered, grouping.ungendered);
        checkBruteForceAvsA(grouping.undirected);
    }

    void ConnectionCheck::checkForConnected(const std::shared_ptr<etree::MeshNode>& nodeA,
                                            const std::shared_ptr<etree::MeshNode>& nodeB) {
        checkForConnected(*getConnectorsOfNode(nodeA),
                          *getConnectorsOfNode(nodeB),
                          glm::transpose(nodeA->getAbsoluteTransformation()),
                          glm::transpose(nodeB->getAbsoluteTransformation()));
    }

    void ConnectionCheck::checkForConnected(const connector_container_t& connectorsA,
                                            const connector_container_t& connectorsB,
                                            const glm::mat4& transfA,
                                            const glm::mat4& transfB) {
        DoubleConnectorGrouping grouping;
        grouping.a.transformation = transfA;
        grouping.b.transformation = transfB;
        grouping.a.addAll(connectorsA);
        grouping.b.addAll(connectorsB);
        grouping.a.updateContainers();
        grouping.b.updateContainers();

        checkDirectionalAvsB(grouping.directions.gendered, grouping.a.gendered[M_IDX], grouping.b.gendered[F_IDX], grouping.a.transformation, grouping.b.transformation);
        checkDirectionalAvsB(grouping.directions.gendered, grouping.b.gendered[M_IDX], grouping.a.gendered[F_IDX], grouping.b.transformation, grouping.a.transformation);
        checkDirectionalAvsB(grouping.directions.ungendered, grouping.a.ungendered, grouping.b.ungendered, grouping.a.transformation, grouping.b.transformation);
        checkBruteForceAvsB(grouping.a.undirected, grouping.b.undirected, grouping.a.transformation, grouping.b.transformation);
    }

    void ConnectionCheck::checkDirectionalAvsA(const DirectionSet& directions,
                                               const std::vector<connector_container_t>& connectors) {
        if (!connectors.empty()) {
            for (std::size_t i = 0; i < directions.size(); ++i) {
                const auto& conns = connectors[i];
                if (i > 2 || conns.size() < 4) {
                    checkBruteForceAvsA(conns);
                } else {
                    checkDirectionalAdvancedAvsA(i, conns);
                }
            }
        }
    }

    void ConnectionCheck::checkDirectionalAvsB(const DirectionSet& directions,
                                               const std::vector<connector_container_t>& aConns,
                                               const std::vector<connector_container_t>& bConns,
                                               const glm::mat4& aTransf,
                                               const glm::mat4& bTransf) {
        if (!aConns.empty() && !bConns.empty()) {
            for (std::size_t i = 0; i < directions.size(); ++i) {
                if (i > 2 || aConns[i].size() * bConns[i].size() < 16) {
                    checkBruteForceAvsB(aConns[i], bConns[i], aTransf, bTransf);
                } else {
                    checkDirectionalAdvancedAvsB(i, aConns[i], bConns[i], aTransf, bTransf);
                }
            }
        }
    }

    void ConnectionCheck::checkDirectionalAdvancedAvsA(size_t directionIdx,
                                                       const connector_container_t& conns) {
        uomap_t<int64_t, uomap_t<int64_t, connector_container_t>> connsByStart;
        const auto ia = directionIdx == 0 ? 1 : 0;
        const auto ib = directionIdx == 2 ? 1 : 2;
        for (const auto& item: conns) {
            connsByStart[item->start[ia]][item->start[ib]].push_back(item);
        }
        for (const auto& i0: connsByStart) {
            for (const auto& i1: i0.second) {
                const auto& sameStartConns = i1.second;
                checkBruteForceAvsA(sameStartConns);
            }
        }
    }

    void ConnectionCheck::checkDirectionalAdvancedAvsB(size_t directionIdx,
                                                       const connector_container_t& aConns,
                                                       const connector_container_t& bConns,
                                                       const glm::mat4& aTransf,
                                                       const glm::mat4& bTransf) {
        const auto iSmaller = aConns.size() < bConns.size() ? 0 : 1;
        const auto& cSmaller = iSmaller == 0 ? aConns : bConns;
        const auto& cLarger = iSmaller == 0 ? bConns : aConns;
        const auto& transfSmaller = iSmaller == 0 ? aTransf : bTransf;
        const auto& transfLarger = iSmaller == 0 ? bTransf : aTransf;
        uomap_t<std::pair<int64_t, int64_t>, std::vector<std::pair<std::shared_ptr<Connector>, PairCheckData>>> connsByStart;//todo dedicated class for this, maybe also for non-axisaligned directions
        const auto ia = directionIdx == 0 ? 1 : 0;
        const auto ib = directionIdx == 2 ? 1 : 2;
        for (const auto& connSmaller: cSmaller) {
            const auto absStart = transfSmaller * glm::vec4(connSmaller->start, 1.f);
            const auto absDirection = transfSmaller * glm::vec4(connSmaller->direction, 0.f);
            connsByStart[{absStart[ia], absStart[ib]}].emplace_back(connSmaller, PairCheckData(transfSmaller, connSmaller, absStart, absDirection));
        }
        for (const auto& connLarger: cLarger) {
            const auto absStart = transfLarger * glm::vec4(connLarger->start, 1.f);
            const auto absDirection = transfLarger * glm::vec4(connLarger->direction, 0.f);
            PairCheckData lData(transfLarger, connLarger, absStart, absDirection);
            if (const auto it0 = connsByStart.find({absStart[ia], absStart[ib]}); it0 != connsByStart.end()) {
                for (const auto& [connSmaller, sData]: it0->second) {
                    PairChecker pc(lData, sData, result);
                    pc.findConnections();
                }
            }
        }
    }

    void ConnectionCheck::checkBruteForceAvsA(const connector_container_t& conns) {
        for (std::size_t i = 0; i < conns.size(); ++i) {
            PairCheckData iData(glm::mat4(1.f), conns[i]);
            for (std::size_t j = 0; j < i; ++j) {
                PairCheckData jData(glm::mat4(1.f), conns[j]);
                PairChecker pc(iData, jData, result);
                pc.findConnections();
            }
        }
    }

    void ConnectionCheck::checkBruteForceAvsB(const connector_container_t& connsA,
                                              const connector_container_t& connsB,
                                              const glm::mat4& aTransf,
                                              const glm::mat4& bTransf) {
        if (!connsA.empty() && !connsB.empty()) {
            for (const auto& a: connsA) {
                PairCheckData aData(aTransf, a);
                for (const auto& b: connsB) {
                    PairCheckData bData(bTransf, b);
                    PairChecker pc(aData, bData, result);
                    pc.findConnections();
                }
            }
        }
    }
}
