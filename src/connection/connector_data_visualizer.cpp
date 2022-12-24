#include "connector_data_visualizer.h"
#include "../ldr/file_repo.h"
#include "../graphics/mesh/mesh_generated.h"
#include "connector_data_provider.h"

namespace bricksim::connection::visualization {

    std::shared_ptr<etree::Node> generateVisualization(const std::string &partName) {
        const auto root = std::make_shared<etree::RootNode>();

        const auto ldrFile = ldr::file_repo::get().getFile(partName);
        const auto partNode = std::make_shared<etree::PartNode>(ldrFile, 1, root, nullptr);
        root->addChild(partNode);

        const auto &connectors = getConnectorsOfPart(partName);
        for (const auto &conn: connectors) {
            const auto clipConn = std::dynamic_pointer_cast<connection::ClipConnector>(conn);
            const auto cylConn = std::dynamic_pointer_cast<connection::CylindricalConnector>(conn);
            const auto fgrConn = std::dynamic_pointer_cast<connection::FingerConnector>(conn);
            const auto genConn = std::dynamic_pointer_cast<connection::GenericConnector>(conn);
            if (clipConn != nullptr) {
            } else if (cylConn != nullptr) {
                const auto c1 = std::make_shared<mesh::generated::CylinderNode>(2, root);
                c1->setRelativeTransformation(conn->location);
                root->addChild(c1);
            } else if (fgrConn != nullptr) {
            } else if (genConn != nullptr) {
            }
        }

        root->incrementVersion();
        return root;
    }
}
