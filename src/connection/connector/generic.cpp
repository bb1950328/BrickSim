#include "generic.h"
#include "../../helpers/stringutil.h"
#include "magic_enum.hpp"
#include <spdlog/fmt/fmt.h>
namespace bricksim::connection {
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
}
