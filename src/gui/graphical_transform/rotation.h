#pragma once

#include "base_action.h"

namespace bricksim::graphical_transform {
    class Rotation : public BaseAction {
    public:
        Rotation(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes);
        ~Rotation() override;
        constexpr GraphicalTransformationType getType() const override;

    protected:
        void startImpl() override;
        void updateImpl() override;
        void endImpl() override;
    };
}
