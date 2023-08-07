#pragma once
#include "../bounding.h"
#include "connector.h"
namespace bricksim::connection {
    class GenericConnector : public Connector {
    public:
        Gender gender;
        bounding_variant_t bounding;

        GenericConnector(const std::string& group,
                         const glm::vec3& start,
                         const glm::vec3& direction,
                         std::string sourceTrace,
                         Gender gender,
                         const bounding_variant_t& bounding);
        std::shared_ptr<Connector> clone() override;
        std::shared_ptr<Connector> transform(const glm::mat4& transformation) override;
        std::string infoStr() const override;
        bool operator==(const GenericConnector& rhs) const;
        bool operator!=(const GenericConnector& rhs) const;
    };
}
