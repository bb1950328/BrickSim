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

    VisualizationGenerator::VisualizationGenerator(const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& fileName) :
        root(std::make_shared<etree::RootNode>()), nameSpace(nameSpace), fileName(fileName) {}
    VisualizationGenerator::VisualizationGenerator(const std::shared_ptr<etree::Node>& container, const std::shared_ptr<ldr::FileNamespace>& nameSpace, const std::string& fileName) :
        root(container), nameSpace(nameSpace), fileName(fileName) {
    }

    const std::shared_ptr<etree::Node>& VisualizationGenerator::getContainer() const {
        return root;
    }
    void VisualizationGenerator::addXYZLineNode() {
        const auto xyzLineNode = std::make_shared<mesh::generated::XYZLineNode>(root);
        xyzLineNode->setRelativeTransformation(glm::transpose(glm::scale(glm::mat4(1.f), glm::vec3(100.f))));
        root->addChild(xyzLineNode);
        root->incrementVersion();
    }
    void VisualizationGenerator::addFileNode() {
        const auto ldrFile = ldr::file_repo::get().getFile(nameSpace, fileName);
        if (ldrFile->metaInfo.type == ldr::FileType::MODEL || ldrFile->metaInfo.type == ldr::FileType::MPD_SUBFILE) {
            const auto modelNode = std::make_shared<etree::ModelNode>(ldrFile, NODE_COLOR, root);
            modelNode->createChildNodes();
            root->addChild(modelNode);
            root->addChild(std::make_shared<etree::ModelInstanceNode>(modelNode, NODE_COLOR, root, nullptr));
        } else {
            root->addChild(std::make_shared<etree::PartNode>(ldrFile, NODE_COLOR, root, nullptr));
        }
        root->incrementVersion();
    }
    void VisualizationGenerator::addConnectorNodes() {
        for (const auto& conn: *getConnectorsOfLdrFile(nameSpace, fileName)) {
            switch (conn->type) {
                case Connector::Type::CYLINDRICAL: {
                    generateCylindrical(std::dynamic_pointer_cast<CylindricalConnector>(conn));
                    break;
                }
                case Connector::Type::CLIP: {
                    generateClip(std::dynamic_pointer_cast<ClipConnector>(conn));
                    break;
                }
                case Connector::Type::FINGER: {
                    generateFinger(std::dynamic_pointer_cast<FingerConnector>(conn));
                    break;
                }
                case Connector::Type::GENERIC: {
                    generateGeneric(std::dynamic_pointer_cast<GenericConnector>(conn));
                    break;
                }
            }
        }
        root->incrementVersion();
    }

    void VisualizationGenerator::generateCylindrical(std::shared_ptr<CylindricalConnector> connector) {
        const auto directionAsQuat = geometry::quaternionRotationFromOneVectorToAnother({0.f, 0.f, 1.f}, connector->direction);
        const glm::vec3 end = connector->start + connector->direction * connector->totalLength;
        {
            const glm::vec3 center = (connector->start + end) / 2.f;
            glm::mat4 transf(1.f);
            transf = glm::toMat4(directionAsQuat) * transf;
            transf = glm::translate(transf, center * directionAsQuat);
            transf = glm::scale(transf, {1.f, 1.f, connector->totalLength});
            const auto centerCylinder = std::make_shared<mesh::generated::CylinderNode>(4, root);
            centerCylinder->setRelativeTransformation(glm::transpose(transf));
            root->addChild(centerCylinder);
        }
        float currentOffset = 0.f;
        for (const auto& part: connector->parts) {
            for (int i = 0; i < 2; ++i) {
                const auto currentCenter = connector->direction * (currentOffset + i * part.length) + connector->start;
                auto transf = glm::mat4(1.f);
                transf = glm::toMat4(directionAsQuat) * transf;
                transf = glm::translate(transf, currentCenter * directionAsQuat);
                transf = glm::scale(transf, glm::vec3(part.radius * 2, part.radius * 2, 1.f));
                const auto c1 = std::make_shared<LineSunNode>(root,
                                                              connector->gender == Gender::F ? FEMALE_CYL_COLOR : MALE_CYL_COLOR,
                                                              connector->gender == Gender::F,
                                                              part.type);
                c1->setRelativeTransformation(glm::transpose(transf));
                root->addChild(c1);
            }
            currentOffset += part.length;
        }
    }
    void VisualizationGenerator::generateClip(std::shared_ptr<ClipConnector> connector) {
        const auto directionAsQuat = geometry::quaternionRotationFromOneVectorToAnother({0.f, 0.f, 1.f}, connector->direction);
        const glm::vec3 end = connector->start + connector->direction * connector->width;
        for (int i = 0; i < 2; ++i) {
            const auto currentCenter = i == 0 ? connector->start : end;
            auto transf = glm::mat4(1.f);
            transf = glm::toMat4(directionAsQuat) * transf;
            transf = glm::translate(transf, currentCenter * directionAsQuat);
            transf = glm::scale(transf, glm::vec3(connector->radius * 2, connector->radius * 2, 1.f));
            const auto c1 = std::make_shared<LineSunNode>(root,
                                                          CLIP_COLOR,
                                                          true,
                                                          CylindricalShapeType::ROUND);
            c1->setRelativeTransformation(glm::transpose(transf));
            root->addChild(c1);
        }
    }
    void VisualizationGenerator::generateFinger(std::shared_ptr<FingerConnector> connector) {
        const auto directionAsQuat = geometry::quaternionRotationFromOneVectorToAnother({0.f, 0.f, 1.f}, connector->direction);
        const glm::vec3 end = connector->start + connector->direction * connector->totalWidth;
        {
            const glm::vec3 center = (connector->start + end) / 2.f;
            glm::mat4 transf(1.f);
            transf = glm::toMat4(directionAsQuat) * transf;
            transf = glm::translate(transf, center * directionAsQuat);
            transf = glm::scale(transf, {1.f, 1.f, connector->totalWidth});
            const auto centerCylinder = std::make_shared<mesh::generated::CylinderNode>(4, root);
            centerCylinder->setRelativeTransformation(glm::transpose(transf));
            root->addChild(centerCylinder);
        }
        float currentOffset = 0.f;
        for (std::size_t i = 0; i <= connector->fingerWidths.size(); ++i) {
            for (const auto gender: {Gender::M, Gender::F}) {
                if (i == 0 && gender != connector->firstFingerGender) {
                    continue;
                }
                if (i == connector->fingerWidths.size()
                    && (i % 2 == 0 ^ gender != connector->firstFingerGender)) {
                    continue;
                }
                const auto currentCenter = connector->direction * currentOffset + connector->start;
                auto transf = glm::mat4(1.f);
                transf = glm::toMat4(directionAsQuat) * transf;
                transf = glm::translate(transf, currentCenter * directionAsQuat);
                transf = glm::scale(transf, glm::vec3(connector->radius * 2, connector->radius * 2, 1.f));
                const auto c1 = std::make_shared<LineSunNode>(root,
                                                              FINGER_COLOR,
                                                              gender == Gender::F,
                                                              CylindricalShapeType::ROUND);
                c1->setRelativeTransformation(glm::transpose(transf));
                root->addChild(c1);
            }
            if (i < connector->fingerWidths.size()) {
                currentOffset += connector->fingerWidths[i];
            }
        }
    }
    void VisualizationGenerator::generateGeneric(std::shared_ptr<GenericConnector> connector) {
        const auto directionAsQuat = geometry::quaternionRotationFromOneVectorToAnother({1.f, 0.f, 0.f}, connector->direction);
        auto transf = glm::mat4(1.f);
        transf = glm::toMat4(directionAsQuat) * transf;
        transf = glm::translate(transf, connector->start * directionAsQuat);

        std::shared_ptr<etree::MeshNode> marker;

        if (std::holds_alternative<BoundingPnt>(connector->bounding)) {
            marker = std::make_shared<PointMarkerNode>(root, GENERIC_COLOR);
            transf = glm::scale(transf, glm::vec3(10.f, 10.f, 10.f));
        } else if (std::holds_alternative<BoundingBox>(connector->bounding)) {
            const auto box = std::get<BoundingBox>(connector->bounding);
            marker = std::make_shared<LineBoxNode>(root, GENERIC_COLOR);
            transf = glm::scale(transf, box.radius * 2.001f);
        } else if (std::holds_alternative<BoundingCube>(connector->bounding)) {
            const auto cube = std::get<BoundingCube>(connector->bounding);
            marker = std::make_shared<LineBoxNode>(root, GENERIC_COLOR);
            transf = glm::scale(transf, glm::vec3(cube.radius, cube.radius, cube.radius) * 2.001f);
        } else if (std::holds_alternative<BoundingCyl>(connector->bounding)) {
            const auto cyl = std::get<BoundingCyl>(connector->bounding);
            marker = std::make_shared<LineCylinderNode>(root, GENERIC_COLOR);
            transf = glm::scale(transf, glm::vec3(2 * cyl.radius, cyl.length, 2 * cyl.radius));
        } else if (std::holds_alternative<BoundingSph>(connector->bounding)) {
            const auto sph = std::get<BoundingSph>(connector->bounding);
            marker = std::make_shared<LineUVSphereNode>(root, GENERIC_COLOR);
            transf = glm::scale(transf, glm::vec3(sph.radius, sph.radius, sph.radius));
        }

        if (marker != nullptr) {
            marker->setRelativeTransformation(glm::transpose(transf));
            root->addChild(marker);
        }
    }
}
