#include "data.h"

#include <utility>

namespace bricksim::connection {

    void ConnectionGraph::addConnection(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b, const ConnectionGraph::edge_t& edge) {
        for (auto& item: getBothVectors(a, b)) {
            item.push_back(edge);
        }
    }
    void ConnectionGraph::removeAllConnections(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) {
        adjacencyLists[a].erase(b);
        adjacencyLists[b].erase(a);
    }
    const std::vector<ConnectionGraph::edge_t>& ConnectionGraph::getConnections(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) {
        const auto it = adjacencyLists.find(a);
        if (it != adjacencyLists.end()) {
            const auto it2 = it->second.find(b);
            if (it2 != it->second.end()) {
                return it2->second;
            }
        }
        const static std::vector<ConnectionGraph::edge_t> empty;
        return empty;
    }
    void ConnectionGraph::removeConnection(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b, const ConnectionGraph::edge_t& edge) {
        for (auto& vec: getBothVectors(a, b)) {
            const auto it = std::find(vec.begin(), vec.end(), edge);
            if (it != vec.end()) {
                vec.erase(it);
            }
        }
    }
    std::array<std::vector<ConnectionGraph::edge_t>, 2> ConnectionGraph::getBothVectors(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) {
        return {adjacencyLists[a][b], adjacencyLists[b][a]};
    }

    Connection::Connection(size_t connectorA, size_t connectorB, DegreesOfFreedom degreesOfFreedom) :
        connectorA(connectorA),
        connectorB(connectorB),
        degreesOfFreedom(std::move(degreesOfFreedom)) {}

    CylindricalConnector::CylindricalConnector(std::string group,
                                               const glm::vec3& start,
                                               const glm::vec3& direction,
                                               Gender gender,
                                               std::vector<CylindricalShapePart> parts,
                                               bool openStart,
                                               bool openEnd,
                                               bool slide) :
        Connector(std::move(group), start),
        direction(direction),
        gender(gender),
        parts(std::move(parts)),
        openStart(openStart),
        openEnd(openEnd),
        slide(slide) {}

    float CylindricalConnector::getTotalLength() const {
        float result = 0;
        for (const auto& item: parts) {
            result += item.length;
        }
        return result;
    }

    Connector::Connector(std::string group, const glm::vec3& start) :
        group(std::move(group)), start(start) {}

    ClipConnector::ClipConnector(const std::string& group,
                                 const glm::vec3& start,
                                 const glm::vec3& direction,
                                 float radius,
                                 float width,
                                 bool slide) :
        Connector(group, start),
        direction(direction), radius(radius), width(width), slide(slide) {}
}
