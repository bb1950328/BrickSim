#include "mesh_collection.h"
#include "../../controller.h"
#include "../../helpers/geometry.h"
#include "../../metrics.h"
#include "../texmap_projection.h"
#include <palanteer.h>
#include <spdlog/spdlog.h>

namespace bricksim::mesh {
    uomap_t<mesh_key_t, std::shared_ptr<Mesh>> SceneMeshCollection::allMeshes;

    mesh_key_t SceneMeshCollection::getMeshKey(const std::shared_ptr<etree::MeshNode>& node, bool windingOrderInverse, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        return {
                node->getMeshIdentifier(),
                windingOrderInverse,
                texmap == nullptr
                        ? 0
                        : robin_hood::hash<ldr::TexmapStartCommand>()(*texmap),
        };
    }

    std::shared_ptr<Mesh> SceneMeshCollection::getMesh(mesh_key_t key, const std::shared_ptr<etree::MeshNode>& node, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        auto it = allMeshes.find(key);
        if (it == allMeshes.end()) {
            plScope("node->addToMesh");
            auto mesh = std::make_shared<Mesh>();
            allMeshes[key] = mesh;
            mesh->name = node->getDescription();
            node->addToMesh(mesh, key.windingInversed, texmap);
            mesh->writeGraphicsData();
            return mesh;
        }
        return it->second;
    }

    std::shared_ptr<etree::Node> SceneMeshCollection::getElementById(element_id_t id) const {
        if (elementsSortedById.size() > id) {
            return elementsSortedById[id];
        }
        return nullptr;
    }

    void SceneMeshCollection::drawLineGraphics(const layer_t layer) const {
        for (const auto& mesh: usedMeshes) {
            mesh->getLineData().draw(mesh->getSceneLayerInstanceRange(scene, layer));
        }
    }

    void SceneMeshCollection::drawOptionalLineGraphics(const layer_t layer) const {
        for (const auto& mesh: usedMeshes) {
            mesh->getOptionalLineData().draw(mesh->getSceneLayerInstanceRange(scene, layer));
        }
    }

    void SceneMeshCollection::drawTriangleGraphics(const layer_t layer) const {
        for (const auto& mesh: usedMeshes) {
            mesh->drawTriangleGraphics(scene, layer);
        }
    }

    void SceneMeshCollection::drawTexturedTriangleGraphics(const layer_t layer) const {
        for (const auto& mesh: usedMeshes) {
            mesh->drawTexturedTriangleGraphics(scene, layer);
        }
    }

    SceneMeshCollection::SceneMeshCollection(scene_id_t scene) :
        scene(scene) {}

    void SceneMeshCollection::readElementTree(const std::shared_ptr<etree::Node>& node,
                                              const glm::mat4& parentAbsoluteTransformation,
                                              std::optional<ldr::ColorReference> parentColor,
                                              std::optional<unsigned int> selectionTargetElementId,
                                              const std::shared_ptr<ldr::TexmapStartCommand>& parentTexmap) {
        std::shared_ptr<etree::Node> nodeToParseChildren = node;
        glm::mat4 absoluteTransformation = parentAbsoluteTransformation * node->getRelativeTransformation();
        std::shared_ptr<ldr::TexmapStartCommand> texmap = parentTexmap != nullptr ? graphics::texmap_projection::transformTexmapStartCommand(parentTexmap, node->getRelativeTransformation()) : nullptr;
        if (node->visible) {
            if ((static_cast<uint32_t>(node->getType()) & static_cast<uint32_t>(etree::NodeType::TYPE_MESH)) > 0) {
                std::shared_ptr<etree::MeshNode> meshNode;
                ldr::ColorReference color;
                std::shared_ptr<etree::MeshNode> nodeToGetColorFrom;
                if (node->getType() == etree::NodeType::TYPE_MPD_SUBFILE_INSTANCE) {
                    const auto instanceNode = std::dynamic_pointer_cast<etree::MpdSubfileInstanceNode>(node);
                    meshNode = instanceNode->mpdSubfileNode;
                    absoluteTransformation = instanceNode->getRelativeTransformation() * parentAbsoluteTransformation;
                    nodeToGetColorFrom = instanceNode;
                    nodeToParseChildren = meshNode;
                } else {
                    meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node);
                    absoluteTransformation = node->getRelativeTransformation() * parentAbsoluteTransformation;
                    nodeToGetColorFrom = meshNode;
                }
                if (nodeToGetColorFrom->getElementColor().get()->code == ldr::Color::MAIN_COLOR_CODE && parentColor.has_value()) {
                    color = parentColor.value();
                } else {
                    color = nodeToGetColorFrom->getDisplayColor();
                }

                if (meshNode->getDirectTexmap() != nullptr) {
                    texmap = meshNode->getDirectTexmap();
                }
                if (texmap != nullptr) {
                    texmap = graphics::texmap_projection::transformTexmapStartCommand(texmap, glm::transpose(node->getRelativeTransformation()));
                }

                if (node->getType() == etree::NodeType::TYPE_MPD_SUBFILE_INSTANCE) {
                    parentColor = color;
                }

                //spdlog::debug("getting mesh key for {}", meshNode->getDescription());
                auto meshKey = getMeshKey(meshNode, geometry::doesTransformationInverseWindingOrder(absoluteTransformation), texmap);
                auto mesh = getMesh(meshKey, meshNode, texmap);
                unsigned int elementId;
                if (selectionTargetElementId.has_value()) {
                    elementId = selectionTargetElementId.value();
                } else {
                    elementId = static_cast<unsigned int>(elementsSortedById.size());
                    if (node->getType() == etree::NodeType::TYPE_MPD_SUBFILE_INSTANCE) {
                        selectionTargetElementId = elementId;//for the children
                    }
                }
                MeshInstance newInstance{color, absoluteTransformation, elementId, meshNode->selected, node->layer, this->scene};
                layersInUse.emplace(node->layer);
                usedMeshes.insert(mesh);
                elementsSortedById.push_back(node);
                newMeshInstances[meshKey].push_back(newInstance);
            }
            for (const auto& child: nodeToParseChildren->getChildren()) {
                if (child->visible) {
                    readElementTree(child, absoluteTransformation, parentColor, selectionTargetElementId, texmap);
                }
            }
            nodesWithChildrenAlreadyVisited.insert(nodeToParseChildren);
        }
    }

    void SceneMeshCollection::rereadElementTreeIfNeeded() {
        plFunction();
        if (lastElementTreeReadVersion == rootNode->getVersion()) {
            return;
        }
        elementsSortedById.clear();
        elementsSortedById.push_back(nullptr);
        layersInUse.clear();
        auto before = std::chrono::high_resolution_clock::now();
        lastUsedMeshes = usedMeshes;
        usedMeshes.clear();
        readElementTree(rootNode, glm::mat4(1.0f), {}, std::nullopt, nullptr);
        updateMeshInstances();
        nodesWithChildrenAlreadyVisited.clear();
        for (const auto& mesh: usedMeshes) {
            mesh->writeGraphicsData();
        }
        for (const auto& mesh: lastUsedMeshes) {
            mesh->writeGraphicsData();
        }
        auto after = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
        metrics::lastElementTreeRereadMs = static_cast<float>(duration) / 1000.f;
        lastElementTreeReadVersion = rootNode->getVersion();
    }

    void SceneMeshCollection::updateMeshInstances() {
        for (auto& [meshKey, newInstancesOfThisScene]: newMeshInstances) {
            auto mesh = allMeshes[meshKey];

            std::sort(newInstancesOfThisScene.begin(), newInstancesOfThisScene.end(), [](const auto& a, const auto& b) {
                return a.layer > b.layer;
            });

            mesh->updateInstancesOfScene(scene, newInstancesOfThisScene);
            lastUsedMeshes.erase(mesh);
        }
        for (const auto& notUsedAnymoreMesh: lastUsedMeshes) {
            notUsedAnymoreMesh->deleteInstancesOfScene(scene);
        }
        newMeshInstances.clear();
    }

    AxisAlignedBoundingBox SceneMeshCollection::getAbsoluteAABB(const std::shared_ptr<const etree::MeshNode>& node) const {
        return getRelativeAABB(node).transform(node->getAbsoluteTransformation());
    }

    AxisAlignedBoundingBox SceneMeshCollection::getRelativeAABB(const std::shared_ptr<const etree::MeshNode>& node) const {
        //todo something here is wrong for subfile instances
        glm::mat4 absoluteTransformation = node->getAbsoluteTransformation();
        auto it = allMeshes.find({node->getMeshIdentifier(), false});
        if (it == allMeshes.end()) {
            it = allMeshes.find({node->getMeshIdentifier(), true});
        }
        mesh::AxisAlignedBoundingBox aabb;
        bool first = true;
        if (it != allMeshes.end()) {
            const auto& mesh = it->second;
            const auto& outerDimensions = mesh->getOuterDimensions();
            aabb.includeAABB(outerDimensions->aabb);
        }
        bool isSubfileInstance = node->getType() == etree::NodeType::TYPE_MPD_SUBFILE_INSTANCE;
        const auto& children = isSubfileInstance
                                       ? std::dynamic_pointer_cast<const etree::MpdSubfileInstanceNode>(node)->mpdSubfileNode->getChildren()
                                       : node->getChildren();
        for (const auto& child: children) {
            if (static_cast<uint32_t>(child->getType()) & static_cast<uint32_t>(etree::NodeType::TYPE_MESH)) {
                auto childResult = getRelativeRotatedBBox(std::dynamic_pointer_cast<const etree::MeshNode>(child));
                if (childResult.has_value()) {
                    aabb.includeBBox(childResult.value());
                }
            }
        }

        //std::cout << node->displayName << ": (" << x1 << ", " << y1 << ", " << z1 << "), (" << x2 << ", " << y2 << ", " << z2 << ")" << std::endl;
        return aabb;
    }

    std::optional<RotatedBoundingBox> SceneMeshCollection::getAbsoluteRotatedBBox(const std::shared_ptr<const etree::MeshNode>& node) const {
        const auto relativeAABB = getRelativeAABB(node);
        if (relativeAABB.isDefined()) {
            const auto nodeAbsTransf = glm::transpose(node->getAbsoluteTransformation());
            if (geometry::doesTransformationLeaveAxisParallels(nodeAbsTransf)) {
                return mesh::RotatedBoundingBox(relativeAABB.transform(nodeAbsTransf));
            } else {
                return mesh::RotatedBoundingBox(relativeAABB).transform(nodeAbsTransf);
            }
        }
        return std::nullopt;
    }

    std::optional<RotatedBoundingBox> SceneMeshCollection::getRelativeRotatedBBox(const std::shared_ptr<const etree::MeshNode>& node) const {
        const auto relativeAABB = getRelativeAABB(node);
        if (relativeAABB.isDefined()) {
            const auto nodeRelTransf = glm::transpose(node->getRelativeTransformation());
            if (geometry::doesTransformationLeaveAxisParallels(nodeRelTransf)) {
                return mesh::RotatedBoundingBox(relativeAABB.transform(nodeRelTransf));
            } else {
                return mesh::RotatedBoundingBox(relativeAABB).transform(nodeRelTransf);
            }
        }
        return std::nullopt;
    }

    const oset_t<layer_t>& SceneMeshCollection::getLayersInUse() const {
        return layersInUse;
    }

    const std::shared_ptr<etree::Node>& SceneMeshCollection::getRootNode() const {
        return rootNode;
    }

    void SceneMeshCollection::setRootNode(const std::shared_ptr<etree::Node>& newRootNode) {
        rootNode = newRootNode;
        lastElementTreeReadVersion = rootNode->getVersion() - 1;
    }

    void SceneMeshCollection::deleteAllMeshes() {
        allMeshes.clear();
    }

    const uoset_t<std::shared_ptr<Mesh>>& SceneMeshCollection::getUsedMeshes() const {
        return usedMeshes;
    }
    bool mesh_key_t::operator==(const mesh_key_t& rhs) const = default;

}
