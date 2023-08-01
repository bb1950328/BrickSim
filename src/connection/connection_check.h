#pragma once

#include "data.h"
#include <array>
#include <vector>
namespace bricksim::connection {
    std::vector<std::array<std::shared_ptr<Connector>, 2>> getConnectedConnectors(const std::vector<std::shared_ptr<Connector>>& connectors);
}
