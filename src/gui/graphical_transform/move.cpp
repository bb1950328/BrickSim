#include "move.h"
namespace bricksim::graphical_transform {

    Move::~Move() = default;
    constexpr GraphicalTransformationType Move::getType() const {
        return GraphicalTransformationType::MOVE;
    }
    void Move::startImpl() {
        BaseAction::startImpl();
    }
    void Move::updateImpl() {
        BaseAction::updateImpl();
    }
    void Move::endImpl() {
        BaseAction::endImpl();
    }
    Move::Move(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) :
        BaseAction(editor, nodes) {}
}
