#pragma once

#include "../../graphics/overlay_2d.h"
#include "base_action.h"

namespace bricksim::graphical_transform {
    class Rotation : public BaseAction {
    public:
        Rotation(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes);
        ~Rotation() override;
        [[nodiscard]] constexpr GraphicalTransformationType getType() const override;

    protected:
        void startImpl() override;
        void updateImpl() override;
        void endImpl() override;

    private:
        std::shared_ptr<overlay2d::LineElement> startLine;
        std::shared_ptr<overlay2d::LineElement> currentLine;
        std::shared_ptr<overlay2d::DashedPolyLineElement> arc;

        float lastRotationAngle = NAN;

        [[nodiscard]] uint8_t findRotationAxis(glm::vec3 worldRayDirection) const;
        [[nodiscard]] static std::vector<std::pair<float, float>> getArcAngles(float& rotationAngle);
        [[nodiscard]] overlay2d::DashedPolyLineElement::points_t getPointsForArc(const std::vector<std::pair<float, float>>& arcAngles,
                                                                                 glm::vec3 axisVec,
                                                                                 glm::vec3 relInitialPointScaled) const;
    };
}
