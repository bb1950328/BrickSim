#pragma once

#include "../../element_tree.h"

namespace bricksim::connection::visualization {
    void addVisualization(const std::string& partName, const std::shared_ptr<etree::Node>& root);
    std::shared_ptr<etree::Node> generateVisualization(const std::string& partName);
}
