#pragma once
#include "connector.h"
#include <vector>

namespace bricksim::connection {
    class FingerConnector : public Connector {
    public:
        Gender firstFingerGender;
        float radius;
        std::vector<float> fingerWidths;
        float totalWidth;

        FingerConnector(const std::string& group,
                        const glm::vec3& start,
                        const glm::vec3& direction,
                        std::string sourceTrace,
                        Gender firstFingerGender,
                        float radius,
                        const std::vector<float>& fingerWidths);
        std::shared_ptr<Connector> clone() override;
        std::shared_ptr<Connector> transform(const glm::mat4& transformation) override;
        std::string infoStr() const override;
        bool operator==(const FingerConnector& rhs) const;
        bool operator!=(const FingerConnector& rhs) const;
    };
}
