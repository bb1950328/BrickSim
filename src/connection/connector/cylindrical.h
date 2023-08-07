#pragma once
#include "connector.h"
#include <vector>

namespace bricksim::connection {
    enum class CylindricalShapeType {
        ROUND,
        AXLE,
        SQUARE,
    };

    class CylindricalShapePart {
    public:
        CylindricalShapeType type;
        bool flexibleRadius;
        float radius;
        float length;
        bool operator==(const CylindricalShapePart& rhs) const;
        bool operator!=(const CylindricalShapePart& rhs) const;
        CylindricalShapePart(CylindricalShapeType type, bool flexibleRadius, float radius, float length);
    };

    class CylindricalConnector : public Connector {
    public:
        Gender gender;
        std::vector<CylindricalShapePart> parts;
        bool openStart;
        bool openEnd;
        bool slide;
        float totalLength;

        CylindricalConnector(const std::string& group,
                             const glm::vec3& start,
                             const glm::vec3& direction,
                             std::string sourceTrace,
                             Gender gender,
                             std::vector<CylindricalShapePart> parts,
                             bool openStart,
                             bool openEnd,
                             bool slide);

        /**
         * @param offsetFromStart
         * @return
         * if the offset is out of bounds, this function will just return the first or last shape part
         */
        [[nodiscard]] const CylindricalShapePart& getPartAt(float offsetFromStart) const;
        [[nodiscard]] float getRadiusAt(float offsetFromStart) const;
        std::shared_ptr<Connector> clone() override;
        std::shared_ptr<Connector> transform(const glm::mat4& transformation) override;
        std::string infoStr() const override;
        size_t hash() const override;
        bool operator==(const CylindricalConnector& rhs) const;
        bool operator!=(const CylindricalConnector& rhs) const;
    };
}

namespace std {
    template<>
    struct hash<bricksim::connection::CylindricalShapePart> {
        std::size_t operator()(const bricksim::connection::CylindricalShapePart& value) const;
    };

}
