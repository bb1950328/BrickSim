#include "finger.h"
#include "../../helpers/stringutil.h"
#include "../../helpers/util.h"
#include "magic_enum.hpp"
#include <numeric>
#include <spdlog/fmt/ranges.h>
namespace bricksim::connection {
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
        Connector(Type::FINGER, group, start, direction, sourceTrace),
        firstFingerGender(firstFingerGender),
        radius(radius),
        fingerWidths(fingerWidths),
        totalWidth(std::reduce(fingerWidths.begin(), fingerWidths.end())) {
    }
    std::string FingerConnector::infoStr() const {
        return fmt::format("finger[group={}, firstFingerGender={}, radius={}, fingerWidths={}, start={}, direction={}]", group, magic_enum::enum_name(firstFingerGender), radius, fingerWidths, stringutil::formatGLM(start), stringutil::formatGLM(direction));
    }
    bool FingerConnector::operator==(const FingerConnector& rhs) const {
        return static_cast<const Connector&>(*this) == static_cast<const Connector&>(rhs)
               && firstFingerGender == rhs.firstFingerGender
               && std::fabs(radius - rhs.radius) < .1f
               && util::floatRangeEpsilonEqual(fingerWidths.cbegin(), fingerWidths.cend(), rhs.fingerWidths.cbegin(), rhs.fingerWidths.cend(), .1f);
    }
    bool FingerConnector::operator!=(const FingerConnector& rhs) const {
        return !(rhs == *this);
    }
    std::shared_ptr<Connector> FingerConnector::transform(const glm::mat4& transformation) {
        const auto [radiusFactor, lengthFactor] = getRadiusAndLengthFactorFromTransformation(transformation, direction);
        std::vector<float> resultFingerWidths;
        resultFingerWidths.reserve(fingerWidths.size());
        for (const auto& w: fingerWidths) {
            resultFingerWidths.push_back(lengthFactor * w);
        }
        return std::make_shared<FingerConnector>(group,
                                                 glm::vec4(start, 1.f) * transformation,
                                                 glm::vec4(direction, 0.f) * transformation,
                                                 sourceTrace,
                                                 firstFingerGender,
                                                 radiusFactor * radius,
                                                 resultFingerWidths);
    }

}
