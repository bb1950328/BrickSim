#include "cylindrical.h"
#include "../../helpers/stringutil.h"
#include "../../helpers/util.h"
#include "magic_enum.hpp"
#include <numeric>
#include <spdlog/fmt/fmt.h>
namespace bricksim::connection {
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

    CylindricalConnector::CylindricalConnector(const std::string& group,
                                               const glm::vec3& start,
                                               const glm::vec3& direction,
                                               std::string sourceTrace,
                                               Gender gender,
                                               std::vector<CylindricalShapePart> parts,
                                               bool openStart,
                                               bool openEnd,
                                               bool slide) :
        Connector(Type::CYLINDRICAL, group, start, direction, sourceTrace),
        gender(gender),
        parts(std::move(parts)),
        openStart(openStart),
        openEnd(openEnd),
        slide(slide),
        totalLength(std::accumulate(this->parts.cbegin(), this->parts.cend(), 0.f,
                                    [](float x, const CylindricalShapePart& p) {
                                        return x + p.length;
                                    })) {
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
        return fmt::format("cylindrical[gender={}, group={}, openStart={}, openEnd={}, slide={}, start={}, direction={}, parts=[{}]]", magic_enum::enum_name(gender), group, openStart, openEnd, slide, stringutil::formatGLM(start), stringutil::formatGLM(direction), pstr);
    }
    bool CylindricalConnector::operator==(const CylindricalConnector& rhs) const {
        return static_cast<const bricksim::connection::Connector&>(*this) == static_cast<const bricksim::connection::Connector&>(rhs)
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
}
namespace std {
    std::size_t hash<bricksim::connection::CylindricalShapePart>::operator()(const bricksim::connection::CylindricalShapePart& value) const {
        return bricksim::util::combinedHash(value.type, value.length, value.radius, value.flexibleRadius);
    }
}
