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

    CylindricalConnector::CylindricalConnector(std::string group,
                                               const glm::vec3& start,
                                               const glm::vec3& direction,
                                               Gender gender,
                                               std::vector<CylindricalShapePart> parts,
                                               bool openStart,
                                               bool openEnd,
                                               bool slide) :
        Connector(std::move(group), start, direction),
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
    std::shared_ptr<Connector> CylindricalConnector::clone() {
        return std::make_shared<CylindricalConnector>(*this);
    }
    glm::vec3 CylindricalConnector::getEnd() const {
        return start + direction * getTotalLength();
    }
    float CylindricalConnector::getRadiusAt(float offsetFromStart) const {
        for (const auto& item: parts) {
            if (item.length < offsetFromStart) {
                return item.radius;
            }
            offsetFromStart -= item.length;
        }
        return NAN;
    }
    const CylindricalShapePart& CylindricalConnector::getPartAt(float offsetFromStart) const {
        for (const auto& item: parts) {
            if (item.length < offsetFromStart) {
                return item;
            }
            offsetFromStart -= item.length;
        }
        return parts.back();
    }

    Connector::Connector(std::string group, const glm::vec3& start, const glm::vec3& direction) :
        group(std::move(group)), start(start), direction(direction) {}

    std::shared_ptr<Connector> Connector::clone() {
        return std::make_shared<Connector>(*this);
    }

    ClipConnector::ClipConnector(const std::string& group,
                                 const glm::vec3& start,
                                 const glm::vec3& direction,
                                 float radius,
                                 float width,
                                 bool slide) :
        Connector(group, start, direction), radius(radius), width(width), slide(slide) {}
    std::shared_ptr<Connector> ClipConnector::clone() {
        return std::make_shared<ClipConnector>(*this);
    }
    std::shared_ptr<Connector> FingerConnector::clone() {
        return std::make_shared<FingerConnector>(*this);
    }
    FingerConnector::FingerConnector(const std::string& group, const glm::vec3& start, const glm::vec3& direction, Gender firstFingerGender, float radius, const std::vector<float>& fingerWidths) :
        Connector(group, start, direction), firstFingerGender(firstFingerGender), radius(radius), fingerWidths(fingerWidths) {}
    std::shared_ptr<Connector> GenericConnector::clone() {
        return std::make_shared<GenericConnector>(*this);
    }
    GenericConnector::GenericConnector(const std::string& group, const glm::vec3& start, const glm::vec3& direction, Gender gender, const bounding_variant_t& bounding) :
        Connector(group, start, direction), gender(gender), bounding(bounding) {}
    DegreesOfFreedom::DegreesOfFreedom(const std::vector<glm::vec3>& slideDirections, const std::vector<glm::vec3>& rotationAxes) :
        slideDirections(slideDirections), rotationAxes(rotationAxes) {}
    DegreesOfFreedom::DegreesOfFreedom() {}
    Connection::Connection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, const DegreesOfFreedom& degreesOfFreedom) :
        connectorA(connectorA), connectorB(connectorB), degreesOfFreedom(degreesOfFreedom) {}
    Connection::Connection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB) :
        connectorA(connectorA), connectorB(connectorB), degreesOfFreedom() {}
}
