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

        std::pair<uint8_t, glm::vec3> findBestPointOnPlanes(glm::usvec2 cursorPos);
    };
}
