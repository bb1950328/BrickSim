// mesh_collection.cpp
// Created by bb1950328 on 03.10.20.
//

#include "mesh_collection.h"
#include "statistic.h"
#include "helpers/util.h"

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
