#include "data.h"
#include "../helpers/geometry.h"
#include "magic_enum.hpp"
#include <glm/gtx/string_cast.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h>

#include <numeric>
#include <utility>

namespace std {
    template<>
    struct hash<bricksim::connection::CylindricalShapePart> {
        std::size_t operator()(const bricksim::connection::CylindricalShapePart& value) const {
            return bricksim::util::combinedHash(value.type, value.length, value.radius, value.flexibleRadius);
        }
    };
}

namespace bricksim::connection {
    namespace {
        std::pair<float, float> getRadiusAndLengthFactorFromTransformation(const glm::mat4& transformation, const glm::vec3& direction) {
            if (std::abs(std::abs(transformation[0][0]) - 1.f) < .001f
                && std::abs(std::abs(transformation[1][1]) - 1.f) < .001f
                && std::abs(std::abs(transformation[2][2]) - 1.f) < .001f) {
                return {1.f, 1.f};
            }
            const auto normalizedDir = glm::normalize(direction);
            const glm::vec3 transfDir = glm::vec4(normalizedDir, 0.f) * transformation;
            const auto perpendicularVector = glm::normalize(geometry::getAnyPerpendicularVector(normalizedDir));
            const glm::vec3 transformedPerpendicular = glm::vec4(perpendicularVector, 0.f) * transformation;
            return {
                    glm::length(transformedPerpendicular),
                    glm::length(transfDir),
            };
        }
    }

    void ConnectionGraph::addConnection(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b, const ConnectionGraph::edge_t& edge) {
        std::lock_guard<std::mutex> lg(lock);
        for (auto& vec: getBothVectors(a, b)) {
            vec.get().push_back(edge);
        }
    }
    void ConnectionGraph::removeAllConnections(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) {
        std::lock_guard<std::mutex> lg(lock);
        adjacencyLists[a].erase(b);
        adjacencyLists[b].erase(a);
    }
    void ConnectionGraph::removeAllConnections(const ConnectionGraph::node_t& a) {
        std::lock_guard<std::mutex> lg(lock);
        adjacencyLists.erase(a);
        for (auto& [x, adj]: adjacencyLists) {
            adj.erase(a);
        }
    }
    void ConnectionGraph::removeAllConnections(const uoset_t<ConnectionGraph::node_t>& toRemove) {
        std::lock_guard<std::mutex> lg(lock);
        if (toRemove.empty()) {
            return;
        }
        for (const auto& a: toRemove) {
            adjacencyLists.erase(a);
        }
        for (auto& [x, adj]: adjacencyLists) {
            for (const auto& a: toRemove) {
                adj.erase(a);
            }
        }
    }

    const std::vector<ConnectionGraph::edge_t>& ConnectionGraph::getConnections(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) const {
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
        std::lock_guard<std::mutex> lg(lock);
        for (auto& vec_wrapper: getBothVectors(a, b)) {
            auto& vec = vec_wrapper.get();
            const auto it = std::find(vec.begin(), vec.end(), edge);
            if (it != vec.end()) {
                vec.erase(it);
            }
        }
    }
    std::array<std::reference_wrapper<std::vector<ConnectionGraph::edge_t>>, 2> ConnectionGraph::getBothVectors(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) {
        return {adjacencyLists[a][b], adjacencyLists[b][a]};
    }
    const uomap_t<ConnectionGraph::node_t, std::vector<ConnectionGraph::edge_t>>& ConnectionGraph::getConnections(const ConnectionGraph::node_t& node) const {
        const auto it = adjacencyLists.find(node);
        if (it == adjacencyLists.end()) {
            const static uomap_t<node_t, std::vector<edge_t>> empty;
            return empty;
        }
        return it->second;
    }
    std::size_t ConnectionGraph::countTotalConnections() const {
        std::size_t total = 0;
        for (const auto& i: adjacencyLists) {
            for (const auto& j: i.second) {
                total += j.second.size();
            }
        }
        return total / 2;
    }
    const bricksim::connection::ConnectionGraph::adjacency_list_t& ConnectionGraph::getAdjacencyLists() const {
        return adjacencyLists;
    }
    void ConnectionGraph::findRestOfClique(uoset_t<ConnectionGraph::node_t>& nodes, const ConnectionGraph::node_t& current) const {
        nodes.insert(current);

        const auto& currentNeighbors = adjacencyLists.find(current)->second;
        for (const auto& n: currentNeighbors) {
            if (!n.second.empty() && !nodes.contains(n.first)) {
                findRestOfClique(nodes, n.first);
            }
        }
    }
    std::vector<uoset_t<ConnectionGraph::node_t>> ConnectionGraph::findAllCliques() const {
        std::vector<uoset_t<node_t>> result;
        uoset_t<node_t> unprocessed;
        unprocessed.reserve(adjacencyLists.size());
        std::transform(adjacencyLists.cbegin(), adjacencyLists.cend(),
                       std::inserter(unprocessed, unprocessed.end()),
                       [](auto entry) { return entry.first; });

        while (!unprocessed.empty()) {
            const auto n = *unprocessed.begin();
            unprocessed.erase(n);
            uoset_t<node_t> clique;
            findRestOfClique(clique, n);
            for (const auto& c: clique) {
                unprocessed.erase(c);
            }
            result.push_back(clique);
        }

        return result;
    }

    CylindricalConnector::CylindricalConnector(const std::string& group,
                                               const glm::vec3& start,
                                               const glm::vec3& direction,
                                               std::string sourceTrace,
                                               Gender gender,
                                               std::vector<CylindricalShapePart> parts,
                                               bool openStart,
                                               bool openEnd,
                                               bool slide) :
        ConnectorWithLength(Type::CYLINDRICAL, group, start, direction, sourceTrace),
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
    std::string CylindricalConnector::infoStr() const {
        std::string pstr;
        for (const auto& p: parts) {
            pstr.append(fmt::format("{}[r={}, fr={}, l={}], ", magic_enum::enum_name(p.type), p.radius, p.flexibleRadius, p.length));
        }
        pstr.pop_back();
        pstr.pop_back();
        return fmt::format("cylindrical[parts=[{}], group={}, openStart={}, openEnd={}, slide={}, start={}, direction={}]", pstr, group, openStart, openEnd, slide, stringutil::formatGLM(start), stringutil::formatGLM(direction));
    }
    bool CylindricalConnector::operator==(const CylindricalConnector& rhs) const {
        return static_cast<const bricksim::connection::ConnectorWithLength&>(*this) == static_cast<const bricksim::connection::ConnectorWithLength&>(rhs)
               && gender == rhs.gender
               && parts == rhs.parts
               && openStart == rhs.openStart
               && openEnd == rhs.openEnd
               && slide == rhs.slide;
    }
    bool CylindricalConnector::operator!=(const CylindricalConnector& rhs) const {
        return !(rhs == *this);
    }
    size_t CylindricalConnector::hash() const {
        return util::combinedHash(Connector::hash(), gender, parts, openStart, openEnd, slide);
    }
    std::shared_ptr<Connector> CylindricalConnector::transform(const glm::mat4& transformation) {
        const auto [radiusFactor, lengthFactor] = getRadiusAndLengthFactorFromTransformation(transformation, direction);

        std::vector<CylindricalShapePart> transformedParts;
        transformedParts.reserve(parts.size());
        for (const auto& item: parts) {
            transformedParts.emplace_back(item.type, item.flexibleRadius, radiusFactor * item.radius, lengthFactor * item.length);
        }
        const glm::vec3 transformedStart = glm::vec4(start, 1.f) * transformation;
        const glm::vec3 transformedDirection = glm::vec4(direction, 0.f) * transformation;
        return std::make_shared<CylindricalConnector>(group,
                                                      transformedStart,
                                                      transformedDirection,
                                                      sourceTrace,
                                                      gender,
                                                      transformedParts,
                                                      openStart,
                                                      openEnd,
                                                      slide);
    }

    Connector::Connector(Type type,
                         std::string group,
                         const glm::vec3& start,
                         const glm::vec3& direction,
                         std::string sourceTrace) :
        type(type),
        group(std::move(group)),
        start(start),
        direction(glm::normalize(direction)),
        sourceTrace(sourceTrace) {}

    std::shared_ptr<Connector> Connector::clone() {
        return std::make_shared<Connector>(*this);
    }
    std::string Connector::infoStr() const {
        return fmt::format("connector[group={}, start={}, direction={}]", group, stringutil::formatGLM(start), stringutil::formatGLM(direction));
    }
    std::size_t Connector::hash() const {
        return util::combinedHash(group, stringutil::formatGLM(start), stringutil::formatGLM(direction));
    }
    bool Connector::operator==(const Connector& rhs) const {
        return group == rhs.group
               && glm::all(glm::epsilonEqual(start, rhs.start, .1f))
               && glm::all(glm::epsilonEqual(direction, rhs.direction, .1f));
    }
    bool Connector::operator!=(const Connector& rhs) const {
        return !(rhs == *this);
    }
    std::shared_ptr<Connector> Connector::transform(const glm::mat4& transformation) {
        return std::make_shared<Connector>(type,
                                           group,
                                           glm::vec4(start, 1.f) * transformation,
                                           glm::vec4(direction, 0.f) * transformation,
                                           sourceTrace);
    }

    ClipConnector::ClipConnector(const std::string& group,
                                 const glm::vec3& start,
                                 const glm::vec3& direction,
                                 std::string sourceTrace,
                                 float radius,
                                 float width,
                                 bool slide,
                                 const glm::vec3& openingDirection) :
        ConnectorWithLength(Type::CLIP, group, start, direction, sourceTrace),
        radius(radius),
        width(width),
        slide(slide),
        openingDirection(openingDirection) {
    }
    std::shared_ptr<Connector> ClipConnector::clone() {
        return std::make_shared<ClipConnector>(*this);
    }
    std::string ClipConnector::infoStr() const {
        return fmt::format("clip[group={}, radius={}, width={}, slide={}, start={}, direction={}]", group, radius, width, slide, stringutil::formatGLM(start), stringutil::formatGLM(direction));
    }
    float ClipConnector::getTotalLength() const {
        return width;
    }
    bool ClipConnector::operator==(const ClipConnector& rhs) const {
        return static_cast<const bricksim::connection::ConnectorWithLength&>(*this) == static_cast<const bricksim::connection::ConnectorWithLength&>(rhs)
               && std::fabs(radius - rhs.radius) < .1f
               && std::fabs(width - rhs.width) < .1f
               && slide == rhs.slide;
    }
    bool ClipConnector::operator!=(const ClipConnector& rhs) const {
        return !(rhs == *this);
    }
    std::shared_ptr<Connector> ClipConnector::transform(const glm::mat4& transformation) {
        const auto [radiusFactor, lengthFactor] = getRadiusAndLengthFactorFromTransformation(transformation, direction);
        return std::make_shared<ClipConnector>(group,
                                               glm::vec4(start, 1.f) * transformation,
                                               glm::vec4(direction, 0.f) * transformation,
                                               sourceTrace,
                                               radiusFactor * radius,
                                               lengthFactor * width,
                                               slide,
                                               glm::vec4(openingDirection, 0.f) * transformation);
    }
    std::shared_ptr<Connector> FingerConnector::clone() {
        return std::make_shared<FingerConnector>(*this);
    }
    FingerConnector::FingerConnector(const std::string& group,
                                     const glm::vec3& start,
                                     const glm::vec3& direction,
                                     std::string sourceTrace,
                                     Gender firstFingerGender,
                                     float radius,
                                     const std::vector<float>& fingerWidths) :
        ConnectorWithLength(Type::FINGER, group, start, direction, sourceTrace),
        firstFingerGender(firstFingerGender),
        radius(radius),
        fingerWidths(fingerWidths) {
    }
    std::string FingerConnector::infoStr() const {
        return fmt::format("finger[group={}, firstFingerGender={}, radius={}, fingerWidths={}, start={}, direction={}]", group, magic_enum::enum_name(firstFingerGender), radius, fingerWidths, stringutil::formatGLM(start), stringutil::formatGLM(direction));
    }
    float FingerConnector::getTotalLength() const {
        return std::reduce(fingerWidths.begin(), fingerWidths.end());
    }
    bool FingerConnector::operator==(const FingerConnector& rhs) const {
        return static_cast<const bricksim::connection::ConnectorWithLength&>(*this) == static_cast<const bricksim::connection::ConnectorWithLength&>(rhs)
               && firstFingerGender == rhs.firstFingerGender
               && std::fabs(radius - rhs.radius) < .1f
               && util::floatRangeEpsilonEqual(fingerWidths.cbegin(), fingerWidths.cend(), rhs.fingerWidths.cbegin(), rhs.fingerWidths.cend(), .1f);
    }
    bool FingerConnector::operator!=(const FingerConnector& rhs) const {
        return !(rhs == *this);
    }
    std::shared_ptr<Connector> FingerConnector::transform(const glm::mat4& transformation) {
        const auto [radiusFactor, lengthFactor] = getRadiusAndLengthFactorFromTransformation(transformation, direction);
        const auto lengthFactorX = lengthFactor;//workaround for clang bug https://www.reddit.com/r/LLVM/comments/s0ykcj/comment/jazmf9m/?utm_source=share&utm_medium=web2x&context=3
        std::vector<float> resultFingerWidths;
        std::transform(fingerWidths.cbegin(),
                       fingerWidths.cend(),
                       resultFingerWidths.begin(),
                       [lengthFactorX](auto w) { return lengthFactorX * w; });
        return std::make_shared<FingerConnector>(group,
                                                 glm::vec4(start, 1.f) * transformation,
                                                 glm::vec4(direction, 0.f) * transformation,
                                                 sourceTrace,
                                                 firstFingerGender,
                                                 radiusFactor * radius,
                                                 resultFingerWidths);
    }

    std::shared_ptr<Connector> GenericConnector::clone() {
        return std::make_shared<GenericConnector>(*this);
    }
    GenericConnector::GenericConnector(const std::string& group,
                                       const glm::vec3& start,
                                       const glm::vec3& direction,
                                       std::string sourceTrace,
                                       Gender gender,
                                       const bounding_variant_t& bounding) :
        Connector(Connector::Type::GENERIC, group, start, direction, sourceTrace),
        gender(gender),
        bounding(bounding) {
    }
    std::string GenericConnector::infoStr() const {
        return fmt::format("generic[group={}, gender={}, bounding={}, start={}, direction={}]", group, magic_enum::enum_name(gender), bounding.index(), stringutil::formatGLM(start), stringutil::formatGLM(direction));
    }
    bool GenericConnector::operator==(const GenericConnector& rhs) const {
        return static_cast<const bricksim::connection::Connector&>(*this) == static_cast<const bricksim::connection::Connector&>(rhs)
               && gender == rhs.gender
               && bounding == rhs.bounding;
    }
    bool GenericConnector::operator!=(const GenericConnector& rhs) const {
        return !(rhs == *this);
    }
    std::shared_ptr<Connector> GenericConnector::transform(const glm::mat4& transformation) {
        return std::make_shared<GenericConnector>(group,
                                                  glm::vec4(start, 1.f) * transformation,
                                                  glm::vec4(direction, 0.f) * transformation,
                                                  sourceTrace,
                                                  gender,
                                                  bounding);
    }
    DegreesOfFreedom::DegreesOfFreedom(const std::vector<glm::vec3>& slideDirections,
                                       const std::vector<RotationPossibility>& rotationPossibilities) :
        slideDirections(slideDirections),
        rotationPossibilities(rotationPossibilities) {
    }
    bool DegreesOfFreedom::operator==(const DegreesOfFreedom& rhs) const {
        return util::vecVecEpsilonEqual(slideDirections, rhs.slideDirections, .01f)
               && rotationPossibilities == rhs.rotationPossibilities;
    }
    bool DegreesOfFreedom::operator!=(const DegreesOfFreedom& rhs) const {
        return !(rhs == *this);
    }
    DegreesOfFreedom::DegreesOfFreedom() = default;
    Connection::Connection(const std::shared_ptr<Connector>& connectorA,
                           const std::shared_ptr<Connector>& connectorB,
                           DegreesOfFreedom degreesOfFreedom) :
        connectorA(connectorA),
        connectorB(connectorB),
        degreesOfFreedom(std::move(degreesOfFreedom)) {
    }
    Connection::Connection(const std::shared_ptr<Connector>& connectorA,
                           const std::shared_ptr<Connector>& connectorB) :
        connectorA(connectorA),
        connectorB(connectorB),
        degreesOfFreedom() {
    }
    bool Connection::operator==(const Connection& rhs) const {
        return connectorA == rhs.connectorA && connectorB == rhs.connectorB && degreesOfFreedom == rhs.degreesOfFreedom;
    }
    bool Connection::operator!=(const Connection& rhs) const {
        return !(rhs == *this);
    }
    glm::vec3 ConnectorWithLength::getEnd() const {
        return start + glm::normalize(direction) * getTotalLength();
    }
    ConnectorWithLength::ConnectorWithLength(Connector::Type type,
                                             const std::string& group,
                                             const glm::vec3& start,
                                             const glm::vec3& direction,
                                             std::string sourceTrace) :
        Connector(type, group, start, direction, sourceTrace) {}
    RotationPossibility::RotationPossibility(const glm::vec3& origin, const glm::vec3& axis) :
        origin(origin), axis(axis) {}
    bool RotationPossibility::operator==(const RotationPossibility& rhs) const {
        return glm::all(glm::epsilonEqual(origin, rhs.origin, .1f))
               && glm::all(glm::epsilonEqual(axis, rhs.axis, .01f));
    }
    bool RotationPossibility::operator!=(const RotationPossibility& rhs) const {
        return !(rhs == *this);
    }

    bool CylindricalShapePart::operator==(const CylindricalShapePart& rhs) const {
        return type == rhs.type
               && flexibleRadius == rhs.flexibleRadius
               && radius == rhs.radius
               && length == rhs.length;
    }
    bool CylindricalShapePart::operator!=(const CylindricalShapePart& rhs) const {
        return !(rhs == *this);
    }
    CylindricalShapePart::CylindricalShapePart(CylindricalShapeType type, bool flexibleRadius, float radius, float length) :
        type(type), flexibleRadius(flexibleRadius), radius(radius), length(length) {}
}
