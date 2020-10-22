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

void MeshCollection::readElementTree(etree::Node *node, const glm::mat4 &parentAbsoluteTransformation) {
    etree::Node* nodeToParseChildren = node;
    glm::mat4 absoluteTransformation = parentAbsoluteTransformation;
    if (node->visible) {
        if ((node->getType() & etree::TYPE_MESH) > 0) {
            etree::MeshNode *meshNode;
            LdrColor *color;
            if (node->getType()==etree::TYPE_MPD_SUBFILE_INSTANCE) {
                const auto instanceNode = dynamic_cast<etree::MpdSubfileInstanceNode *>(node);
                meshNode = instanceNode->mpdSubfileNode;
                absoluteTransformation = instanceNode->getRelativeTransformation()*parentAbsoluteTransformation;
                color = instanceNode->color;
                nodeToParseChildren = meshNode;
            } else {
                meshNode = dynamic_cast<etree::MeshNode *>(node);
                absoluteTransformation = node->getRelativeTransformation() * parentAbsoluteTransformation;
                color = meshNode->color;
            }
            void *identifier = meshNode->getMeshIdentifier();
            bool windingInversed = util::doesTransformationInverseWindingOrder(absoluteTransformation);
            auto it = meshes.find(std::make_pair(identifier, windingInversed));
            Mesh *mesh;
            if (it != meshes.end()) {
                mesh = it->second;
            } else {
                mesh = new Mesh();
                meshes[std::make_pair(identifier, windingInversed)] = mesh;
                mesh->name = meshNode->getDescription();
                meshNode->addToMesh(mesh, windingInversed);
            }
            const auto elementId = static_cast<unsigned int>(elementsSortedById.size());
            MeshInstance newInstance{color, absoluteTransformation, elementId};
            elementsSortedById.push_back(node);
            auto instIdxKeyPair = std::make_pair(meshNode, windingInversed);
            auto instIdxIterator = meshInstanceIndices.find(instIdxKeyPair);
            if (instIdxIterator!=meshInstanceIndices.end()) {
                if (mesh->instances[instIdxIterator->second] != newInstance) {
                    mesh->instances[instIdxIterator->second] = newInstance;
                    mesh->instancesHaveChanged = true;
                }
            } else {
                meshInstanceIndices[instIdxKeyPair] = mesh->instances.size();
                mesh->instances.push_back(newInstance);
                mesh->instancesHaveChanged = true;
            }
        }
        if (nodesWithChildrenAlreadyVisited.find(nodeToParseChildren)==nodesWithChildrenAlreadyVisited.end()) {
            for (const auto &child: nodeToParseChildren->getChildren()) {
                if (child->visible) {
                    readElementTree(child, absoluteTransformation);
                }
            }
            nodesWithChildrenAlreadyVisited.insert(nodeToParseChildren);
        }
    }
}

MeshCollection::MeshCollection(etree::ElementTree *elementTree) {
    this->elementTree = elementTree;
}

void MeshCollection::rereadElementTree() {
    elementsSortedById.clear();
    elementsSortedById.push_back(nullptr);
    auto before = std::chrono::high_resolution_clock::now();
    readElementTree(&elementTree->rootNode, glm::mat4(1.0f));
    nodesWithChildrenAlreadyVisited.clear();
    for (const auto &mesh: meshes) {
        mesh.second->writeGraphicsData();
    }
    auto after = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
    std::cout << "rereadElementTree() in " << duration / 1000.0f << "ms" << std::endl;
}

etree::Node *MeshCollection::getElementById(unsigned int id) {
    if (elementsSortedById.size()>id) {
        return elementsSortedById[id];
    }
    return nullptr;
}
