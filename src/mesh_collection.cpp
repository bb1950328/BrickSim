// mesh_collection.cpp
// Created by bb1950328 on 03.10.20.
//

#include "mesh_collection.h"
#include "statistic.h"
#include "util.h"

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

void MeshCollection::readElementTree(ElementTreeNode *node) {
    if (node->visible) {
        if ((node->getType() & ET_TYPE_MESH) > 0) { // todo google if there's something like instanceof in C++
            auto *meshNode = dynamic_cast<ElementTreeMeshNode *>(node);
            void *identifier = meshNode->getMeshIdentifier();
            const glm::mat4 &absoluteTransformation = node->getAbsoluteTransformation();
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
            MeshInstance newInstance{meshNode->color, absoluteTransformation, elementsSortedById.size()};
            elementsSortedById.push_back(node);
            if (meshNode->instanceIndex.has_value()) {
                if (mesh->instances[meshNode->instanceIndex.value()] != newInstance) {
                    mesh->instances[meshNode->instanceIndex.value()] = newInstance;
                    mesh->instancesHaveChanged = true;
                }
            } else {
                meshNode->instanceIndex = std::make_optional(mesh->instances.size());
                mesh->instances.push_back(newInstance);
                mesh->instancesHaveChanged = true;
            }
        }
        for (const auto &child: node->children) {
            if (child->visible) {
                readElementTree(child);
            }
        }
    }
}

MeshCollection::MeshCollection(ElementTree *elementTree) {
    this->elementTree = elementTree;
}

void MeshCollection::rereadElementTree() {
    size_t sizeBefore = elementsSortedById.size();
    elementsSortedById.clear();
    elementsSortedById.resize(sizeBefore);//todo check if this is necessary
    elementsSortedById.push_back(nullptr);
    auto before = std::chrono::high_resolution_clock::now();
    readElementTree(&elementTree->rootNode);
    for (const auto &mesh: meshes) {
        mesh.second->writeGraphicsData();
    }
    auto after = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
    std::cout << "rereadElementTree() in " << duration / 1000.0f << "ms" << std::endl;
}

ElementTreeNode *MeshCollection::getElementById(unsigned int id) {
    return elementsSortedById[id];
}
