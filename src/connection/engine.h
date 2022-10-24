#pragma once

#include "data.h"
namespace bricksim::connection::engine {
    std::vector<Connection> findConnections(const std::shared_ptr<etree::LdrNode>& a, const std::shared_ptr<etree::LdrNode>& b);

}