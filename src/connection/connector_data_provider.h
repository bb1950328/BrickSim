#pragma once

#include "../ldr/files.h"
#include "data.h"
namespace bricksim::connection {
    std::shared_ptr<std::vector<std::shared_ptr<Connector>>> getConnectorsOfPart(const std::string& name);
    std::shared_ptr<std::vector<std::shared_ptr<Connector>>> getConnectorsOfNode(const std::shared_ptr<etree::LdrNode>& node);
}
