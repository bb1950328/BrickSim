#pragma once

#include "data.h"
namespace bricksim::connection::engine {
    constexpr float PARALLELITY_ANGLE_TOLERANCE = .001f;
    constexpr float COLINEARITY_TOLERANCE_LDU = .1f;

    std::vector<Connection> findConnections(const std::shared_ptr<etree::LdrNode>& a, const std::shared_ptr<etree::LdrNode>& b);

}
