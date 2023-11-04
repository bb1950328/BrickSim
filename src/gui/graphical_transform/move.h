#pragma once
#include "../../snapping/snap_to_connector.h"
#include "base_action.h"

namespace bricksim::graphical_transform {
    class Move : public BaseAction {
    public:
        Move(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes);
        ~Move() override;
        [[nodiscard]] constexpr GraphicalTransformationType getType() const override;

    protected:
        std::unique_ptr<snap::SnapToConnectorProcess> snapProcess;

        void startImpl() override;
        void updateImpl() override;
        void endImpl() override;
    };
}
