// mesh_collection.cpp
// Created by bb1950328 on 03.10.20.
//

#include "mesh_collection.h"
#include "statistic.h"
#include "helpers/util.h"
#include "controller.h"

void MeshCollection::initializeGraphics() {
    for (const auto &pair: meshes) {
        pair.second->writeGraphicsData();
    }
}

void MeshCollection::drawLineGraphics() const {
    for (const auto &pair: meshes) {
        pair.second->drawLineGraphics();
    }
}

void MeshCollection::drawOptionalLineGraphics() const {
    for (const auto &pair: meshes) {
        pair.second->drawOptionalLineGraphics();
    }
}

void MeshCollection::drawTriangleGraphics() const {
    for (const auto &pair: meshes) {
        pair.second->drawTriangleGraphics();
    }
}

void MeshCollection::deallocateGraphics() {
    for (const auto &pair: meshes) {
        pair.second->deallocateGraphics();
    }
}

void MeshCollection::readElementTree(etree::Node *node, const glm::mat4 &parentAbsoluteTransformation, LdrColor *parentColor) {
    etree::Node *nodeToParseChildren = node;
    glm::mat4 absoluteTransformation = parentAbsoluteTransformation;
    if (node->visible) {
        if ((node->getType() & etree::TYPE_MESH) > 0) {
            etree::MeshNode *meshNode;
            LdrColor *color;
            etree::MeshNode *nodeToGetColorFrom;
            if (node->getType() == etree::TYPE_MPD_SUBFILE_INSTANCE) {
                const auto instanceNode = dynamic_cast<etree::MpdSubfileInstanceNode *>(node);
                meshNode = instanceNode->mpdSubfileNode;
                absoluteTransformation = instanceNode->getRelativeTransformation() * parentAbsoluteTransformation;
                nodeToGetColorFrom = instanceNode;
                nodeToParseChildren = meshNode;
            } else {
                meshNode = dynamic_cast<etree::MeshNode *>(node);
                absoluteTransformation = node->getRelativeTransformation() * parentAbsoluteTransformation;
                nodeToGetColorFrom = meshNode;
            }
            if (nodeToGetColorFrom->getElementColor()->code == LdrColor::MAIN_COLOR_CODE && parentColor != nullptr) {
                color = parentColor;
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
                Mesh *mesh = new Mesh();
                meshes[meshesKey] = mesh;
                mesh->name = meshNode->getDescription();
                meshNode->addToMesh(mesh, windingInversed);
            }
            const auto elementId = static_cast<unsigned int>(elementsSortedById.size());
            MeshInstance newInstance{color, absoluteTransformation, elementId, meshNode->selected};
            elementsSortedById.push_back(node);
            newMeshInstances[meshesKey].push_back(newInstance);
        }
        for (const auto &child: nodeToParseChildren->getChildren()) {
            if (child->visible) {
                readElementTree(child, absoluteTransformation, parentColor);
            }
        }
        nodesWithChildrenAlreadyVisited.insert(nodeToParseChildren);
    }
}

MeshCollection::MeshCollection(etree::ElementTree *elementTree) {
    this->elementTree = elementTree;
}

void MeshCollection::rereadElementTree() {
    elementsSortedById.clear();
    elementsSortedById.push_back(nullptr);
    auto before = std::chrono::high_resolution_clock::now();
    readElementTree(&elementTree->rootNode, glm::mat4(1.0f), nullptr);
    updateMeshInstances();
    nodesWithChildrenAlreadyVisited.clear();
    for (const auto &mesh: meshes) {
        mesh.second->writeGraphicsData();
    }
    auto after = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
    statistic::lastElementTreeReread = duration / 1000.0f;
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

etree::Node *MeshCollection::getElementById(unsigned int id) {
    if (elementsSortedById.size() > id) {
        return elementsSortedById[id];
    }
    return nullptr;
}

void MeshCollection::updateSelectionContainerBox() {
    static Mesh *selectionBoxMesh = nullptr;
    if (selectionBoxMesh == nullptr) {
        selectionBoxMesh = new Mesh();
        meshes[std::make_pair(reinterpret_cast<void*>(selectionBoxMesh), false)] = selectionBoxMesh;
        selectionBoxMesh->addLdrFile(*ldr_file_repo::get_file("box0.dat"), glm::mat4(1.0f), &ldr_color_repo::getInstanceDummyColor(), false);
    }
    selectionBoxMesh->instances.clear();
    if (!controller::getSelectedNodes().empty()) {
        for (const auto &node : controller::getSelectedNodes()) {
            if (node->getType() & etree::TYPE_MESH) {
                //todo draw selection as line if only one part is selected
                // fix the transformation (click the red 2x4 tile for example)
                auto boxDimensions = controller::getRenderer()->meshCollection.getBoundingBox(dynamic_cast<const etree::MeshNode *>(node));
                auto p1 = boxDimensions.first;
                auto p2 = boxDimensions.second;
                auto center = (p1 + p2) / 2.0f;
                auto size = p1 - p2;
                auto transformation = glm::scale(glm::mat4(1.0f), size / 2.0f) * node->getAbsoluteTransformation();//the /2 is because box0.dat has 2ldu edge length
                selectionBoxMesh->instances.push_back({ldr_color_repo::get_color(1), transformation, 0, true});
            }
        }
    }
    selectionBoxMesh->instancesHaveChanged = true;
    selectionBoxMesh->writeGraphicsData();
    controller::getRenderer()->unrenderedChanges = true;
}

std::pair<glm::vec3, glm::vec3> MeshCollection::getBoundingBox(const etree::MeshNode* node) const {
    glm::mat4 absoluteTransformation = node->getAbsoluteTransformation();
    bool windingInversed = util::doesTransformationInverseWindingOrder(absoluteTransformation);
    auto it = meshes.find(std::make_pair(node->getMeshIdentifier(), windingInversed));
    float x1, x2, y1, y2, z1, z2;
    bool first = true;
    if (it != meshes.end()) {
        Mesh *mesh = it->second;
        for (const auto &lineVertex : mesh->lineVertices) {//todo check if iterating over triangle vertices is faster
            if (first) {
                first = false;
                x1 = x2 = lineVertex.position.x;
                y1 = y2 = lineVertex.position.y;
                z1 = z2 = lineVertex.position.z;
            } else {
                x1 = std::min(x1, lineVertex.position.x);
                x2 = std::max(x2, lineVertex.position.x);
                y1 = std::min(y1, lineVertex.position.y);
                y2 = std::max(y2, lineVertex.position.y);
                z1 = std::min(z1, lineVertex.position.z);
                z2 = std::max(z2, lineVertex.position.z);
            }
        }
    }
    for (const auto &child : node->getChildren()) {
        if (child->getType()&etree::TYPE_MESH) {
            auto childResult = getBoundingBox(dynamic_cast<const etree::MeshNode*>(child));
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
    return {{x1, y1, z1}, {x2, y2, z2}};
}
