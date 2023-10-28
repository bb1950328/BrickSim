#pragma once

#include "../element_tree.h"
#include "../ldr/files.h"
#include "connector_conversion.h"
namespace bricksim::connection {
    void removeConnected(connector_container_t& connectors);
    std::size_t removeDuplicates(connector_container_t& connectors);
    std::shared_ptr<connector_container_t> getConnectorsOfLdrFile(const std::shared_ptr<ldr::FileNamespace>& fileNamespace, const std::string& name);
    std::shared_ptr<connector_container_t> getConnectorsOfNode(const std::shared_ptr<etree::MeshNode>& node);
    std::shared_ptr<connector_container_t> getConnectorsOfLdrFile(const std::shared_ptr<ldr::File>& ldrFile);
}
