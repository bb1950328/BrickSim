#pragma once

#include "../../element_tree.h"

namespace bricksim::connection::visualization {
    void addVisualization(const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& partName, const std::shared_ptr<etree::Node>& root);
    std::shared_ptr<etree::Node> generateVisualization(const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& fileName);
}
