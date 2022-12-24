#include "connector_data_visualizer.h"
#include "../ldr/file_repo.h"

namespace bricksim::connection::visualization {

    std::shared_ptr<etree::Node> generateVisualization(const std::string &partName) {
        const auto root = std::make_shared<etree::RootNode>();
        const auto ldrFile = ldr::file_repo::get().getFile(partName);
        const auto partNode = std::make_shared<etree::PartNode>(ldrFile, 1, root, nullptr);
        root->addChild(partNode);
        root->incrementVersion();
        return root;
    }
}
