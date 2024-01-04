#pragma once

#include "base_action.h"

namespace bricksim::graphical_transform {
    std::unique_ptr<BaseAction> createAction(GraphicalTransformationType type, Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes);
}
