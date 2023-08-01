#include "connector_data_visualizer.h"
#include "../../graphics/mesh/mesh_generated.h"
#include "../../helpers/geometry.h"
#include "../../ldr/file_repo.h"
#include "../connector_data_provider.h"
#include "meshes.h"

namespace bricksim::connection::visualization {
    constexpr ldr::Color::code_t NODE_COLOR = 1;
    std::shared_ptr<etree::Node> generateVisualization(const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& fileName) {
        const auto root = std::make_shared<etree::RootNode>();

        const auto ldrFile = ldr::file_repo::get().getFile(nameSpace, fileName);
        if (ldrFile->metaInfo.type == ldr::FileType::MODEL || ldrFile->metaInfo.type == ldr::FileType::MPD_SUBFILE) {
            const auto modelNode = std::make_shared<etree::ModelNode>(ldrFile, NODE_COLOR, root);
            modelNode->createChildNodes();
            root->addChild(modelNode);
            root->addChild(std::make_shared<etree::ModelInstanceNode>(modelNode, NODE_COLOR, root, nullptr));
        } else {
            root->addChild(std::make_shared<etree::PartNode>(ldrFile, NODE_COLOR, root, nullptr));
        }

        const auto xyzLineNode = std::make_shared<mesh::generated::XYZLineNode>(root);
        xyzLineNode->setRelativeTransformation(glm::transpose(glm::scale(glm::mat4(1.f), glm::vec3(100.f))));
        root->addChild(xyzLineNode);

        addVisualization(nameSpace, fileName, root);

        return root;
    }
    void addVisualization(const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& partName, const std::shared_ptr<etree::Node>& root) {
        for (const auto& conn: *getConnectorsOfLdrFile(nameSpace, partName)) {
            const auto cylConn = std::dynamic_pointer_cast<CylindricalConnector>(conn);
            const auto clipConn = std::dynamic_pointer_cast<ClipConnector>(conn);
            const auto fgrConn = std::dynamic_pointer_cast<FingerConnector>(conn);
            const auto genConn = std::dynamic_pointer_cast<GenericConnector>(conn);
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
                    const auto centerCylinder = std::make_shared<mesh::generated::CylinderNode>(4, root);
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
                        const auto c1 = std::make_shared<LineSunNode>(root, SimpleLineColor::RED, cylConn->gender == Gender::F, part.type);
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
    }
}
