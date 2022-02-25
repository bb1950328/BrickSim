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
            std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
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
            if ((node->getType() & etree::TYPE_MESH) > 0) {
                std::shared_ptr<etree::MeshNode> meshNode;
                ldr::ColorReference color;
                std::shared_ptr<etree::MeshNode> nodeToGetColorFrom;
                if (node->getType() == etree::TYPE_MPD_SUBFILE_INSTANCE) {
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

                if (node->getType() == etree::TYPE_MPD_SUBFILE_INSTANCE) {
                    parentColor = color;
                }

                //spdlog::debug("getting mesh key for {}", meshNode->getDescription());
                auto meshKey = getMeshKey(meshNode, geometry::doesTransformationInverseWindingOrder(absoluteTransformation), texmap);
                auto mesh = getMesh(meshKey, meshNode, texmap);
                unsigned int elementId;
                if (selectionTargetElementId.has_value()) {
                    elementId = selectionTargetElementId.value();
                } else {
                    elementId = elementsSortedById.size();
                    if (node->getType() == etree::TYPE_MPD_SUBFILE_INSTANCE) {
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
        metrics::lastElementTreeRereadMs = duration / 1000.0f;
        lastElementTreeReadVersion = rootNode->getVersion();
    }

    void SceneMeshCollection::updateMeshInstances() {
        for (const auto& pair: newMeshInstances) {
            auto meshKey = pair.first;
            auto newInstancesOfThisScene = pair.second;
            auto mesh = allMeshes[meshKey];

            std::sort(newInstancesOfThisScene.begin(), newInstancesOfThisScene.end(), [](MeshInstance& a, MeshInstance& b) {
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

    /* void SceneMeshCollection::updateSelectionContainerBoxIfNeeded() {
        //todo think about moving this to editor
        static std::shared_ptr<Mesh> selectionBoxMesh = nullptr;
        std::optional<std::shared_ptr<Editor>> editor = controller::getEditorOfScene(scene);
        if (editor.has_value()) {
            if (selectionBoxMesh == nullptr) {
                selectionBoxMesh = std::make_shared<Mesh>();
                usedMeshes.insert(selectionBoxMesh);
                selectionBoxMesh->addLdrFile(ldr::color_repo::getInstanceDummyColor(), ldr::file_repo::get().getFile("box0.dat"), glm::mat4(1.0f), false, nullptr);
            }
            selectionBoxMesh->instances.clear();
            const auto& selectedNodes = editor.value()->getSelectedNodes();
            if (!selectedNodes.empty()) {
                for (const auto& node: selectedNodes) {
                    if (node->getType() & etree::TYPE_MESH) {
                        //todo draw selection as line if only one part is selected
                        // fix the transformation (click the red 2x4 tile for example)
                        auto boxDimensions = getBoundingBoxAbsolute(std::dynamic_pointer_cast<const etree::MeshNode>(node));
                        auto p1 = boxDimensions.first;
                        auto p2 = boxDimensions.second;
                        auto center = (p1 + p2) / 2.0f;
                        auto size = p1 - p2;
                        //std::cout << "------------------------------" << std::endl;
                        //std::cout << "name: " << node->displayName << std::endl;
                        //std::cout << "p1: " << glm::to_string(p1) << std::endl;
                        //std::cout << "p2: " << glm::to_string(p2) << std::endl;
                        //std::cout << "center: " << glm::to_string(center) << std::endl;
                        //std::cout << "size: " << glm::to_string(size) << std::endl;
                        auto transformation = glm::mat4(1.0f);
                        transformation = glm::translate(transformation, center);
                        transformation = glm::scale(transformation, size / 2.0f);//the /2 is because box0.dat has 2ldu edge length
                        transformation = glm::transpose(transformation);
                        selectionBoxMesh->instances.push_back({{1}, transformation, 0, true, 0, scene});//todo update ranges
                    }
                }
            }
            selectionBoxMesh->instancesHaveChanged = true;
            selectionBoxMesh->writeGraphicsData();
        }
    }
    */

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
            aabb.addAABB(outerDimensions->aabb);
        }
        bool isSubfileInstance = node->getType() == etree::TYPE_MPD_SUBFILE_INSTANCE;
        const auto& children = isSubfileInstance
                                       ? std::dynamic_pointer_cast<const etree::MpdSubfileInstanceNode>(node)->mpdSubfileNode->getChildren()
                                       : node->getChildren();
        for (const auto& child: children) {
            if (child->getType() & etree::TYPE_MESH) {
                auto childResult = getRelativeAABB(std::dynamic_pointer_cast<const etree::MeshNode>(child)).transform(child->getRelativeTransformation());
                aabb.addAABB(childResult);
            }
        }

        //std::cout << node->displayName << ": (" << x1 << ", " << y1 << ", " << z1 << "), (" << x2 << ", " << y2 << ", " << z2 << ")" << std::endl;
        return aabb;
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
    bool mesh_key_t::operator==(const mesh_key_t& rhs) const {
        return meshIdentifier == rhs.meshIdentifier && windingInversed == rhs.windingInversed && texmapHash == rhs.texmapHash;
    }
    bool mesh_key_t::operator!=(const mesh_key_t& rhs) const {
        return !(rhs == *this);
    }
}