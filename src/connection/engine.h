#pragma once

#include "../graphics/mesh/mesh_collection.h"
#include "data.h"
namespace bricksim::connection::engine {
    constexpr float PARALLELITY_ANGLE_TOLERANCE = .001f;
    constexpr float COLINEARITY_TOLERANCE_LDU = .1f;
    constexpr float POSITION_TOLERANCE_LDU = .1f;
    constexpr float CYLINDRICAL_CONNECTION_RADIUS_TOLERANCE = 1.f;

    std::vector<std::shared_ptr<Connection>> findConnections(const std::shared_ptr<etree::LdrNode>& a, const std::shared_ptr<etree::LdrNode>& b);
    ConnectionGraph findConnections(const std::shared_ptr<etree::Node>& node, const mesh::SceneMeshCollection& meshCollection);
    ConnectionGraph findConnections(const std::shared_ptr<etree::LdrNode>& activeNode, const std::shared_ptr<etree::Node>& passiveNode, const mesh::SceneMeshCollection& meshCollection);
}