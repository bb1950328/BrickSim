#pragma once
#include "connector.h"

namespace bricksim::connection {
    class ClipConnector : public Connector {
    public:
        float radius;
        float width;
        bool slide;
        glm::vec3 openingDirection;

        ClipConnector(const std::string& group,
                      const glm::vec3& start,
                      const glm::vec3& direction,
                      std::string sourceTrace,
                      float radius,
                      float width,
                      bool slide,
                      const glm::vec3& openingDirection);
        std::shared_ptr<Connector> clone() override;
        std::shared_ptr<Connector> transform(const glm::mat4& transformation) override;
        std::string infoStr() const override;
        bool operator==(const ClipConnector& rhs) const;
        bool operator!=(const ClipConnector& rhs) const;
    };
}
