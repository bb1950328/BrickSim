#pragma once

#include "../ldr/files.h"
#include "data.h"
namespace bricksim::connection {
    struct SharedPtrConnectorValueHash {
        uint64_t operator()(const std::shared_ptr<Connector>& connector) const {
            return std::hash<Connector>()(*connector);
        }
    };
    using connector_container_t = std::vector<std::shared_ptr<Connector>>;//ankerl::unordered_dense::set<std::shared_ptr<Connector>, SharedPtrConnectorValueHash>;
    std::shared_ptr<connector_container_t> getConnectorsOfLdrFile(const std::shared_ptr<ldr::FileNamespace>& fileNamespace, const std::string& name);
    std::shared_ptr<connector_container_t> getConnectorsOfNode(const std::shared_ptr<etree::MeshNode>& node);
    std::shared_ptr<connector_container_t> getConnectorsOfLdrFile(const std::shared_ptr<ldr::File>& ldrFile);
}
