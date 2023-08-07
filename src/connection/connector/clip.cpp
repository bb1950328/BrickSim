#include "clip.h"
#include "../../helpers/stringutil.h"
#include <spdlog/fmt/fmt.h>
namespace bricksim::connection {
    ClipConnector::ClipConnector(const std::string& group,
                                 const glm::vec3& start,
                                 const glm::vec3& direction,
                                 std::string sourceTrace,
                                 float radius,
                                 float width,
                                 bool slide,
                                 const glm::vec3& openingDirection) :
        Connector(Type::CLIP, group, start, direction, sourceTrace),
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
    bool ClipConnector::operator==(const ClipConnector& rhs) const {
        return static_cast<const bricksim::connection::Connector&>(*this) == static_cast<const bricksim::connection::Connector&>(rhs)
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
}
