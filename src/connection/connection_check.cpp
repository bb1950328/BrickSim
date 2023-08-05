#include "connection_check.h"
#include "magic_enum.hpp"
#include "pair_checker.h"
namespace bricksim::connection {
    namespace {
        struct CompareFirst {
            bool operator()(float d, const std::pair<float, std::shared_ptr<Connector>>& p) {
                return p.first < d;
            }
            bool operator()(const std::pair<float, std::shared_ptr<Connector>>& p, float d) {
                return d < p.first;
            }
        };
        void checkDirectionalGenderedBruteForce(std::vector<std::array<std::shared_ptr<Connector>, 2>>& result, const std::array<std::vector<std::shared_ptr<Connector>>, 2>& conns) {
            const auto& mConns = conns[*magic_enum::enum_index(Gender::M)];
            const auto& fConns = conns[*magic_enum::enum_index(Gender::F)];
            for (const auto& m: mConns) {
                PairCheckData mData(glm::mat4(1.f), m);
                for (const auto& f: fConns) {
                    PairCheckData fData(glm::mat4(1.f), f);
                    VectorPairChecker pc(mData, fData, result);
                    pc.findConnections();
                }
            }
        }
        template<typename T>
            requires std::is_base_of_v<Connector, T>
        void checkBruteForce(std::vector<std::array<std::shared_ptr<Connector>, 2>>& result, const std::vector<std::shared_ptr<T>>& conns) {
            for (std::size_t i = 0; i < conns.size(); ++i) {
                PairCheckData iData(glm::mat4(1.f), conns[i]);
                for (std::size_t j = 0; j < i; ++j) {
                    PairCheckData jData(glm::mat4(1.f), conns[j]);
                    VectorPairChecker pc(iData, jData, result);
                    pc.findConnections();
                }
            }
        }
        void checkDirectionalGenderedAdvanced(size_t directionIdx, std::vector<std::array<std::shared_ptr<Connector>, 2>>& result, const std::array<std::vector<std::shared_ptr<Connector>>, 2>& conns) {
            const auto iSmaller = conns[0].size() < conns[1].size() ? 0 : 1;
            const auto& cSmaller = conns[iSmaller];
            const auto& cLarger = conns[1 - iSmaller];
            uomap_t<int64_t, uomap_t<int64_t, std::vector<std::shared_ptr<Connector>>>> connsByStart;
            const auto ia = directionIdx == 0 ? 1 : 0;
            const auto ib = directionIdx == 2 ? 1 : 2;
            for (const auto& item: cSmaller) {
                connsByStart[item->start[ia]][item->start[ib]].push_back(item);
            }
            for (const auto& item: cLarger) {
                PairCheckData lData(glm::mat4(1.f), item);
                if (const auto it0 = connsByStart.find(item->start[ia]); it0 != connsByStart.end()) {
                    if (const auto it1 = it0->second.find(item->start[ib]); it1 != it0->second.end()) {
                        for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
                            PairCheckData sData(glm::mat4(1.f), *it2);
                            VectorPairChecker pc(lData, sData, result);
                            pc.findConnections();
                        }
                    }
                }
            }
        }
        void checkDirectionalUngenderedAdvanced(size_t directionIdx, std::vector<std::array<std::shared_ptr<Connector>, 2>>& result, const std::vector<std::shared_ptr<FingerConnector>>& conns) {
            uomap_t<int64_t, uomap_t<int64_t, std::vector<std::shared_ptr<FingerConnector>>>> connsByStart;
            const auto ia = directionIdx == 0 ? 1 : 0;
            const auto ib = directionIdx == 2 ? 1 : 2;
            for (const auto& item: conns) {
                connsByStart[item->start[ia]][item->start[ib]].push_back(item);
            }
            for (const auto& i0: connsByStart) {
                for (const auto& i1: i0.second) {
                    const auto& sameStartConns = i1.second;
                    checkBruteForce(result, sameStartConns);
                }
            }
        }
        void checkDirectionalGendered(std::vector<std::array<std::shared_ptr<Connector>, 2>>& result, std::vector<std::pair<glm::vec3, std::array<std::vector<std::shared_ptr<Connector>>, 2>>>& cylConnectors) {
            for (std::size_t i = 0; i < cylConnectors.size(); ++i) {
                const auto& conns = cylConnectors[i].second;
                if (i > 2 || conns[0].size() * conns[1].size() < 16) {
                    checkDirectionalGenderedBruteForce(result, conns);
                } else {
                    checkDirectionalGenderedAdvanced(i, result, conns);
                }
            }
        }
        void checkDirectionalUngendered(std::vector<std::array<std::shared_ptr<Connector>, 2>>& result, std::vector<std::pair<glm::vec3, std::vector<std::shared_ptr<FingerConnector>>>>& connectors) {
            for (std::size_t i = 0; i < connectors.size(); ++i) {
                const auto& conns = connectors[i].second;
                if (i > 2 || conns.size() < 4) {
                    checkBruteForce(result, conns);
                } else {
                    checkDirectionalUngenderedAdvanced(i, result, conns);
                }
            }
        }
    }
    std::vector<std::array<std::shared_ptr<Connector>, 2>> getConnectedConnectors(const std::vector<std::shared_ptr<Connector>>& connectors) {
        std::vector<std::array<std::shared_ptr<Connector>, 2>> result;

        std::vector<std::pair<glm::vec3, std::array<std::vector<std::shared_ptr<Connector>>, 2>>> directionalGenderedConnectors = {
                {glm::vec3(1.f, 0.f, 0.f), {}},
                {glm::vec3(0.f, 1.f, 0.f), {}},
                {glm::vec3(0.f, 0.f, 1.f), {}},
        };
        std::vector<std::pair<glm::vec3, std::vector<std::shared_ptr<FingerConnector>>>> directionalUngenderedConnectors = {
                {glm::vec3(1.f, 0.f, 0.f), {}},
                {glm::vec3(0.f, 1.f, 0.f), {}},
                {glm::vec3(0.f, 0.f, 1.f), {}},
        };
        std::vector<std::shared_ptr<Connector>> genericConnectors;

        for (const auto& conn: connectors) {
            auto gender = Gender::F;
            bool foundDir = false;
            switch (conn->type) {
                case Connector::Type::CYLINDRICAL:
                    gender = std::dynamic_pointer_cast<CylindricalConnector>(conn)->gender;
                case Connector::Type::CLIP:
                    for (auto& [dir, connsWithDir]: directionalGenderedConnectors) {
                        if (glm::length(glm::cross(dir, conn->direction)) < PARALLELITY_ANGLE_TOLERANCE) {
                            //add clips to Gender::F because they only match with male cylinders
                            connsWithDir[*magic_enum::enum_index(gender)].push_back(conn);
                            foundDir = true;
                            break;
                        }
                    }
                    if (!foundDir) {
                        directionalGenderedConnectors.push_back({conn->direction, {}});
                        directionalGenderedConnectors.back().second[*magic_enum::enum_index(gender)].push_back(conn);
                    }
                    break;
                case Connector::Type::FINGER:
                    for (auto& [dir, connsWithDir]: directionalUngenderedConnectors) {
                        if (glm::length(glm::cross(dir, conn->direction)) < PARALLELITY_ANGLE_TOLERANCE) {
                            connsWithDir.push_back(std::dynamic_pointer_cast<FingerConnector>(conn));
                            foundDir = true;
                            break;
                        }
                    }
                    if (!foundDir) {
                        directionalUngenderedConnectors.push_back({conn->direction, {}});
                        directionalUngenderedConnectors.back().second.push_back(std::dynamic_pointer_cast<FingerConnector>(conn));
                    }
                    break;
                case Connector::Type::GENERIC:
                    genericConnectors.push_back(conn);
                    break;
                default:
                    break;
            }
        }

        checkDirectionalGendered(result, directionalGenderedConnectors);
        checkDirectionalUngendered(result, directionalUngenderedConnectors);
        checkBruteForce(result, genericConnectors);

        return result;
    }
}
