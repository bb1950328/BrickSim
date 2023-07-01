#include "provider.h"
#include "rotation.h"
#include "translation.h"

namespace bricksim::graphical_transform {

    std::unique_ptr<BaseAction> createAction(GraphicalTransformationType type, Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) {
        switch (type) {
            case GraphicalTransformationType::TRANSLATE: return std::make_unique<Translation>(editor, nodes);
            case GraphicalTransformationType::ROTATE: return std::make_unique<Rotation>(editor, nodes);
            default: throw std::invalid_argument("");
        }
    }
}
