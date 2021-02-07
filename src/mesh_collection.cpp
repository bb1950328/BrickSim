// mesh_collection.cpp
// Created by bb1950328 on 03.10.20.
//

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <utility>
#include "mesh_collection.h"
#include "statistic.h"
#include "helpers/util.h"
#include "controller.h"
#include "ldr_files/ldr_file_repo.h"

void MeshCollection::initializeGraphics() {
    for (const auto &pair: meshes) {
        pair.second->writeGraphicsData();
    }
}

void MeshCollection::drawLineGraphics(const layer_t layer) const {
    for (const auto &pair: meshes) {
        pair.second->drawLineGraphics(layer);
    }
}

void MeshCollection::drawOptionalLineGraphics(const layer_t layer) const {
    for (const auto &pair: meshes) {
        pair.second->drawOptionalLineGraphics(layer);
    }
}

void MeshCollection::drawTriangleGraphics(const layer_t layer) const {
    for (const auto &pair: meshes) {
        pair.second->drawTriangleGraphics(layer);
    }
}

void MeshCollection::deallocateGraphics() {
    /* todo integrate this into destructor
    for (const auto &pair: meshes) {
        pair.second->deallocateGraphics();
    }*/
}

void MeshCollection::readElementTree(const std::shared_ptr<etree::Node>& node, const glm::mat4 &parentAbsoluteTransformation, std::optional<LdrColorReference> parentColor, std::optional<unsigned int> selectionTargetElementId) {
    std::shared_ptr<etree::Node> nodeToParseChildren = node;
    glm::mat4 absoluteTransformation = parentAbsoluteTransformation;
    if (node->visible) {
        if ((node->getType() & etree::TYPE_MESH) > 0) {
            std::shared_ptr<etree::MeshNode> meshNode;
            LdrColorReference color;
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
            if (nodeToGetColorFrom->getElementColor().get()->code == LdrColor::MAIN_COLOR_CODE && parentColor.has_value()) {
                color = parentColor.value();
            } else {
                color = nodeToGetColorFrom->getDisplayColor();
            }

            if (node->getType() == etree::TYPE_MPD_SUBFILE_INSTANCE) {
                parentColor = color;
            }

            void *identifier = meshNode->getMeshIdentifier();
            bool windingInversed = util::doesTransformationInverseWindingOrder(absoluteTransformation);
            auto meshesKey = std::make_pair(identifier, windingInversed);
            auto it = meshes.find(meshesKey);
            if (it == meshes.end()) {
                const std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
                meshes[meshesKey] = mesh;
                mesh->name = meshNode->getDescription();
                meshNode->addToMesh(mesh, windingInversed);
            }
            unsigned int elementId;
            if (selectionTargetElementId.has_value()) {
                elementId = selectionTargetElementId.value();
            } else {
                elementId = static_cast<unsigned int>(elementsSortedById.size());
                if (node->getType() == etree::TYPE_MPD_SUBFILE_INSTANCE) {
                    selectionTargetElementId = elementId;//for the children
                }
            }
            MeshInstance newInstance{color, absoluteTransformation, elementId, meshNode->selected, node->layer};
            layersInUse.emplace(node->layer);
            elementsSortedById.push_back(node);
            newMeshInstances[meshesKey].push_back(newInstance);
        }
        for (const auto &child: nodeToParseChildren->getChildren()) {
            if (child->visible) {
                readElementTree(child, absoluteTransformation, parentColor, selectionTargetElementId);
            }
        }
        nodesWithChildrenAlreadyVisited.insert(nodeToParseChildren);
    }
}

MeshCollection::MeshCollection(std::shared_ptr<etree::ElementTree> elementTree) {
    this->elementTree = std::move(elementTree);
}

void MeshCollection::rereadElementTree() {
    elementsSortedById.clear();
    elementsSortedById.push_back(nullptr);
    layersInUse.clear();
    auto before = std::chrono::high_resolution_clock::now();
    readElementTree(elementTree->rootNode, glm::mat4(1.0f), {}, std::nullopt);
    updateMeshInstances();
    nodesWithChildrenAlreadyVisited.clear();
    for (const auto &mesh: meshes) {
        mesh.second->writeGraphicsData();
    }
    auto after = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
    statistic::lastElementTreeRereadMs = duration / 1000.0f;
}

void MeshCollection::updateMeshInstances() {
    for (const auto &pair : newMeshInstances) {
        auto meshKey = pair.first;
        auto newVector = pair.second;
        auto mesh = meshes[meshKey];
        if (mesh->instances != newVector) {
            mesh->instances = newVector;
            mesh->instancesHaveChanged = true;
        }
    }
    newMeshInstances.clear();
}

std::shared_ptr<etree::Node> MeshCollection::getElementById(unsigned int id) {
    if (elementsSortedById.size() > id) {
        return elementsSortedById[id];
    }
    return nullptr;
}

void MeshCollection::updateSelectionContainerBox() {
    static std::shared_ptr<Mesh> selectionBoxMesh = nullptr;
    if (selectionBoxMesh == nullptr) {
        selectionBoxMesh = std::make_shared<Mesh>();
        meshes[std::make_pair(selectionBoxMesh.get(), false)] = selectionBoxMesh;
        selectionBoxMesh->addLdrFile(ldr_file_repo::get().getFile("box0.dat"), glm::mat4(1.0f), ldr_color_repo::getInstanceDummyColor(), false);
    }
    selectionBoxMesh->instances.clear();
    if (!controller::getSelectedNodes().empty()) {
        for (const auto &node : controller::getSelectedNodes()) {
            if (node->getType() & etree::TYPE_MESH) {
                //todo draw selection as line if only one part is selected
                // fix the transformation (click the red 2x4 tile for example)
                auto boxDimensions = controller::getRenderer()->meshCollection->getBoundingBox(std::dynamic_pointer_cast<const etree::MeshNode>(node));
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
                selectionBoxMesh->instances.push_back({{1}, transformation, 0, true});
            }
        }
    }
    selectionBoxMesh->instancesHaveChanged = true;
    selectionBoxMesh->writeGraphicsData();
    controller::getRenderer()->unrenderedChanges = true;
}

std::pair<glm::vec3, glm::vec3> MeshCollection::getBoundingBox(const std::shared_ptr<const etree::MeshNode>& node) const {
    auto result = getBoundingBoxInternal(node);
    return {
        glm::vec4(result.first, 1.0f) * node->getAbsoluteTransformation(),
        glm::vec4(result.second, 1.0f) * node->getAbsoluteTransformation(),
    };
}

//relative to parameter node
std::pair<glm::vec3, glm::vec3> MeshCollection::getBoundingBoxInternal(std::shared_ptr<const etree::MeshNode> node) const {
    //todo something here is wrong for subfile instances
    glm::mat4 absoluteTransformation;
    absoluteTransformation = node->getAbsoluteTransformation();
    bool windingInversed = util::doesTransformationInverseWindingOrder(absoluteTransformation);
    auto it = meshes.find(std::make_pair(node->getMeshIdentifier(), windingInversed));
    float x1=0, x2=0, y1=0, y2=0, z1=0, z2=0;
    bool first = true;
    if (it != meshes.end()) {
        const auto mesh = it->second;
        for (const auto &colorPair : mesh->triangleVertices) {
            for (const auto &triangleVertex : colorPair.second) {//todo check if iterating over line vertices is faster
                const glm::vec4 &position = triangleVertex.position;
                if (first) {
                    first = false;
                    x1 = x2 = position.x;
                    y1 = y2 = position.y;
                    z1 = z2 = position.z;
                } else {
                    x1 = std::min(x1, position.x);
                    x2 = std::max(x2, position.x);
                    y1 = std::min(y1, position.y);
                    y2 = std::max(y2, position.y);
                    z1 = std::min(z1, position.z);
                    z2 = std::max(z2, position.z);
                }
            }
        }
    }
    bool isSubfileInstance = node->getType() == etree::TYPE_MPD_SUBFILE_INSTANCE;
    const auto &children = isSubfileInstance
            ? std::dynamic_pointer_cast<const etree::MpdSubfileInstanceNode>(node)->mpdSubfileNode->getChildren()
            : node->getChildren();
    for (const auto &child : children) {
        if (child->getType()&etree::TYPE_MESH) {
            auto childResult = getBoundingBoxInternal(std::dynamic_pointer_cast<const etree::MeshNode>(child));
            childResult.first = glm::vec4(childResult.first, 1.0f)*child->getRelativeTransformation();
            childResult.second = glm::vec4(childResult.second, 1.0f)*child->getRelativeTransformation();
            //std::cout << glm::to_string(childResult.first) << ", " << glm::to_string(childResult.second) << std::endl;
            //std::cout << glm::to_string(child->getRelativeTransformation()) << std::endl;
            if (first) {
                first = false;
                x1 = childResult.first.x;
                x2 = childResult.second.x;
                y1 = childResult.first.y;
                y2 = childResult.second.y;
                z1 = childResult.first.z;
                z2 = childResult.second.z;
            } else {
                x1 = std::min(x1, childResult.first.x);
                x2 = std::max(x2, childResult.second.x);
                y1 = std::min(y1, childResult.first.y);
                y2 = std::max(y2, childResult.second.y);
                z1 = std::min(z1, childResult.first.z);
                z2 = std::max(z2, childResult.second.z);
            }
        }
    }

    //std::cout << node->displayName << ": (" << x1 << ", " << y1 << ", " << z1 << "), (" << x2 << ", " << y2 << ", " << z2 << ")" << std::endl;
    return {
        glm::vec4(x1, y1, z1, 1.0f),
        glm::vec4(x2, y2, z2, 1.0f)
    };
}

const std::set<layer_t> &MeshCollection::getLayersInUse() const {
    return layersInUse;
}

MeshCollection::~MeshCollection() {
    deallocateGraphics();
}
