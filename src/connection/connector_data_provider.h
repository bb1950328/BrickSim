#pragma once

#include "../ldr/files.h"
#include "data.h"
namespace bricksim::connection {
    const std::vector<std::shared_ptr<Connector>>& getConnectorsOfPart(const std::string& name);
}