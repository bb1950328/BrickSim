#include "connector.h"
#include "../../helpers/geometry.h"
#include "../../helpers/stringutil.h"

namespace bricksim::connection {
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
    std::pair<float, float> Connector::getRadiusAndLengthFactorFromTransformation(const glm::mat4& transformation, const glm::vec3& direction) {
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
namespace std {
    std::size_t hash<bricksim::connection::Connector>::operator()(const bricksim::connection::Connector& value) const {
        return value.hash();
    }
}
