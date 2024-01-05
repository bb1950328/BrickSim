#include "move.h"
#include "../../controller.h"

namespace bricksim::graphical_transform {
    Move::~Move() = default;

    constexpr GraphicalTransformationType Move::getType() const {
        return GraphicalTransformationType::MOVE;
    }

    void Move::startImpl() {
        if (controller::getSnapHandler().isEnabled()) {
            snapProcess = std::make_unique<snap::SnapToConnectorProcess>(nodes, editor.shared_from_this(), initialCursorPos);
        }
    }

    void Move::updateImpl() {
        if (snapProcess != nullptr) {
            snapProcess->updateCursorPos(currentCursorPos);
            if (snapProcess->getResultCount() > 0) {
                snapProcess->applyResultTransformation(0);
            }
        }
    }

    void Move::endImpl() {}

    Move::Move(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) :
        BaseAction(editor, nodes) {}
}
