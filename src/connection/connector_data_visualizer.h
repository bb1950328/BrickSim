#pragma once

#include "../element_tree.h"

namespace bricksim::connection::visualization {
    std::shared_ptr<etree::Node> generateVisualization(const std::string &partName);
}
