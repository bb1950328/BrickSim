#pragma once
#include "../../element_tree.h"
#include "../../graphics/scene.h"
#include "base_action.h"
#include <memory>

namespace bricksim::graphical_transform {
    namespace o2d = overlay2d;

    class Translation : public BaseAction {
    public:
        explicit Translation(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes);
        ~Translation() override;

        [[nodiscard]] constexpr GraphicalTransformationType getType() const override;

    protected:
        void startImpl() override;
        void updateImpl() override;
        void endImpl() override;

    private:
        std::array<std::shared_ptr<o2d::DashedLineElement>, 3> axisLines;

        glm::vec3 currentNodeCenter;

        glm::vec3 startPoint;
        glm::vec3 currentPoint;
        float distanceToCamera;

        glm::vec3 lastTranslation = {NAN, NAN, NAN};

        void addAxisLines();
        void removeAxisLines();
        void updateAxisLines();
    };
}
