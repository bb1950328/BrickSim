#include "rotation.h"

namespace bricksim::graphical_transform {
    void Rotation::startImpl() {
        BaseAction::startImpl();
    }
    void Rotation::updateImpl() {
        BaseAction::updateImpl();
    }
    void Rotation::endImpl() {
        BaseAction::endImpl();
    }
    Rotation::Rotation(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) :
        BaseAction(editor, nodes) {}
    constexpr GraphicalTransformationType Rotation::getType() const {
        return GraphicalTransformationType::ROTATE;
    }
    Rotation::~Rotation() = default;
}
