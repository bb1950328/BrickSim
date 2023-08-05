#include "connector_data_visualizer.h"
#include "../../graphics/mesh/mesh_generated.h"
#include "../../helpers/geometry.h"
#include "../../ldr/file_repo.h"
#include "../connector_data_provider.h"
#include "meshes.h"

namespace bricksim::connection::visualization {
    constexpr ldr::Color::code_t NODE_COLOR = 15;//White
    constexpr auto MALE_CYL_COLOR = SimpleLineColor::BLUE;
    constexpr auto FEMALE_CYL_COLOR = SimpleLineColor::CYAN;
    constexpr auto CLIP_COLOR = SimpleLineColor::RED;
    constexpr auto FINGER_COLOR = SimpleLineColor::GREEN;
    constexpr auto GENERIC_COLOR = SimpleLineColor::MAGENTA;
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
            const glm::vec3 direction = conn->direction;
            if (cylConn != nullptr) {
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
                        const auto c1 = std::make_shared<LineSunNode>(root,
                                                                      cylConn->gender == Gender::F ? FEMALE_CYL_COLOR : MALE_CYL_COLOR,
                                                                      cylConn->gender == Gender::F,
                                                                      part.type);
                        c1->setRelativeTransformation(glm::transpose(transf));
                        root->addChild(c1);
                    }
                    currentOffset += part.length;
                }
            } else if (clipConn != nullptr) {
                const auto directionAsQuat = geometry::quaternionRotationFromOneVectorToAnother({0.f, 0.f, 1.f}, direction);
                const glm::vec3 end = clipConn->start + direction * clipConn->width;
                for (int i = 0; i < 2; ++i) {
                    const auto currentCenter = i == 0 ? clipConn->start : end;
                    auto transf = glm::mat4(1.f);
                    transf = glm::toMat4(directionAsQuat) * transf;
                    transf = glm::translate(transf, currentCenter * directionAsQuat);
                    transf = glm::scale(transf, glm::vec3(clipConn->radius * 2, clipConn->radius * 2, 1.f));
                    const auto c1 = std::make_shared<LineSunNode>(root,
                                                                  CLIP_COLOR,
                                                                  true,
                                                                  CylindricalShapeType::ROUND);
                    c1->setRelativeTransformation(glm::transpose(transf));
                    root->addChild(c1);
                }
            } else if (fgrConn != nullptr) {
                const auto directionAsQuat = geometry::quaternionRotationFromOneVectorToAnother({0.f, 0.f, 1.f}, direction);
                const glm::vec3 start = fgrConn->start;
                const float totalLength = fgrConn->getTotalLength();
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
                for (std::size_t i = 0; i <= fgrConn->fingerWidths.size(); ++i) {
                    for (const auto gender: {Gender::M, Gender::F}) {
                        if (i == 0 && gender != fgrConn->firstFingerGender) {
                            continue;
                        }
                        if (i == fgrConn->fingerWidths.size()
                            && (i % 2 == 0 ^ gender != fgrConn->firstFingerGender)) {
                            continue;
                        }
                        const auto currentCenter = direction * currentOffset + fgrConn->start;
                        auto transf = glm::mat4(1.f);
                        transf = glm::toMat4(directionAsQuat) * transf;
                        transf = glm::translate(transf, currentCenter * directionAsQuat);
                        transf = glm::scale(transf, glm::vec3(fgrConn->radius * 2, fgrConn->radius * 2, 1.f));
                        const auto c1 = std::make_shared<LineSunNode>(root,
                                                                      FINGER_COLOR,
                                                                      gender == Gender::F,
                                                                      CylindricalShapeType::ROUND);
                        c1->setRelativeTransformation(glm::transpose(transf));
                        root->addChild(c1);
                    }
                    if (i < fgrConn->fingerWidths.size()) {
                        currentOffset += fgrConn->fingerWidths[i];
                    }
                }
            } else if (genConn != nullptr) {
                const auto directionAsQuat = geometry::quaternionRotationFromOneVectorToAnother({1.f, 0.f, 0.f}, direction);
                auto transf = glm::mat4(1.f);
                transf = glm::toMat4(directionAsQuat) * transf;
                transf = glm::translate(transf, genConn->start * directionAsQuat);

                std::shared_ptr<etree::MeshNode> marker;

                if (std::holds_alternative<BoundingPnt>(genConn->bounding)) {
                    marker = std::make_shared<PointMarkerNode>(root, GENERIC_COLOR);
                    transf = glm::scale(transf, glm::vec3(10.f, 10.f, 10.f));
                } else if (std::holds_alternative<BoundingBox>(genConn->bounding)) {
                    const auto box = std::get<BoundingBox>(genConn->bounding);
                    marker = std::make_shared<LineBoxNode>(root, GENERIC_COLOR);
                    transf = glm::scale(transf, box.radius * 2.001f);
                } else if (std::holds_alternative<BoundingCube>(genConn->bounding)) {
                    const auto cube = std::get<BoundingCube>(genConn->bounding);
                    marker = std::make_shared<LineBoxNode>(root, GENERIC_COLOR);
                    transf = glm::scale(transf, glm::vec3(cube.radius, cube.radius, cube.radius) * 2.001f);
                } else if (std::holds_alternative<BoundingCyl>(genConn->bounding)) {
                    const auto cyl = std::get<BoundingCyl>(genConn->bounding);
                    marker = std::make_shared<LineCylinderNode>(root, GENERIC_COLOR);
                    transf = glm::scale(transf, glm::vec3(2 * cyl.radius, cyl.length, 2 * cyl.radius));
                } else if (std::holds_alternative<BoundingSph>(genConn->bounding)) {
                    const auto sph = std::get<BoundingSph>(genConn->bounding);
                    marker = std::make_shared<LineUVSphereNode>(root, GENERIC_COLOR);
                    transf = glm::scale(transf, glm::vec3(sph.radius, sph.radius, sph.radius));
                }

                if (marker != nullptr) {
                    marker->setRelativeTransformation(glm::transpose(transf));
                    root->addChild(marker);
                }
            }
        }

        root->incrementVersion();
    }
}
