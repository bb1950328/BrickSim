#include "connector_data_visualizer.h"
#include "../graphics/mesh/mesh_generated.h"
#include "../helpers/geometry.h"
#include "../ldr/file_repo.h"
#include "connector_data_provider.h"

namespace bricksim::connection::visualization {

    std::shared_ptr<etree::Node> generateVisualization(const std::string& partName) {
        const auto root = std::make_shared<etree::RootNode>();

        const auto ldrFile = ldr::file_repo::get().getFile(partName);
        const auto partNode = std::make_shared<etree::PartNode>(ldrFile, 1, root, nullptr);
        root->addChild(partNode);

        const auto xyzLineNode = std::make_shared<mesh::generated::XYZLineNode>(root);
        xyzLineNode->setRelativeTransformation(glm::transpose(glm::scale(glm::mat4(1.f), glm::vec3(10.f))));
        root->addChild(xyzLineNode);

        for (const auto& conn: getConnectorsOfPart(partName)) {
            const auto cylConn = std::dynamic_pointer_cast<connection::CylindricalConnector>(conn);
            const auto clipConn = std::dynamic_pointer_cast<connection::ClipConnector>(conn);
            const auto fgrConn = std::dynamic_pointer_cast<connection::FingerConnector>(conn);
            const auto genConn = std::dynamic_pointer_cast<connection::GenericConnector>(conn);
            if (cylConn != nullptr) {
                const glm::vec3 direction = cylConn->direction;
                const auto directionAsQuat = geometry::quaternionRotationFromOneVectorToAnother({0.f, 0.f, 1.f}, direction);
                const glm::vec3 start = cylConn->start;
                const float totalLength = cylConn->getTotalLength();
                const glm::vec3 end = start + direction * totalLength;
                {
                    const glm::vec3 center = (start + end) / 2.f;
                    glm::mat4 transf(1.f);
                    transf = glm::toMat4(directionAsQuat) * transf;
                    transf = glm::translate(transf, center * directionAsQuat);
                    transf = glm::scale(transf, {1.f, 1.f, totalLength});
                    const auto centerCylinder = std::make_shared<mesh::generated::CylinderNode>(2, root);
                    centerCylinder->setRelativeTransformation(glm::transpose(transf));
                    root->addChild(centerCylinder);
                }
                float currentOffset = 0.f;
                for (const auto& part: cylConn->parts) {
                    for (int i = 0; i < 2; ++i) {
                        const auto currentCenter = direction * (currentOffset + i * part.length) + cylConn->start;
                        auto transf = glm::mat4(1.f);
                        transf = glm::toMat4(directionAsQuat) * transf;
                        transf = glm::translate(transf, currentCenter * directionAsQuat);
                        transf = glm::scale(transf, glm::vec3(part.radius * 2, part.radius * 2, 1.f));
                        const auto c1 = std::make_shared<mesh::generated::LineSunNode>(root,
                                                                                       mesh::generated::SimpleLineColor::RED);
                        c1->setRelativeTransformation(glm::transpose(transf));
                        root->addChild(c1);
                    }
                    currentOffset += part.length;
                }
            } else if (clipConn != nullptr) {
            } else if (fgrConn != nullptr) {
            } else if (genConn != nullptr) {
            }
        }

        root->incrementVersion();
        return root;
    }
}
