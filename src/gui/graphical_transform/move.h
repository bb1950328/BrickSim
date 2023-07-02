#pragma once
#include "base_action.h"

namespace bricksim::graphical_transform {
    class Move : public BaseAction {
    public:
        Move(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes);
        ~Move() override;
        [[nodiscard]] constexpr GraphicalTransformationType getType() const override;

    protected:
        void startImpl() override;
        void updateImpl() override;
        void endImpl() override;
    };
}
