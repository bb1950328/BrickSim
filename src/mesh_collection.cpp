// mesh_collection.cpp
// Created by bb1950328 on 03.10.20.
//

#include "mesh_collection.h"
#include "statistic.h"

void MeshCollection::initializeGraphics() {
    for (const auto &pair: meshes) {
        pair.second->writeGraphicsData();
    }
}

void MeshCollection::drawLineGraphics(const Shader *lineShader) const {
    for (const auto &pair: meshes) {
        pair.second->drawLineGraphics(lineShader);
    }
}

void MeshCollection::drawTriangleGraphics(const Shader *triangleShader) const {
    for (const auto &pair: meshes) {
        pair.second->drawTriangleGraphics(triangleShader);
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
            auto it = meshes.find(identifier);
            Mesh *mesh;
            if (it != meshes.end()) {
                mesh = it->second;
            } else {
                mesh = new Mesh();
                meshes[identifier] = mesh;
                mesh->name = meshNode->getDescription();
                meshNode->addToMesh(mesh);
            }
            auto newPair = std::make_pair(meshNode->color, node->getAbsoluteTransformation());
            if (meshNode->instanceIndex.has_value()) {
                if (mesh->instances[meshNode->instanceIndex.value()] != newPair) {
                    mesh->instances[meshNode->instanceIndex.value()] = newPair;
                    mesh->instancesHaveChanged = true;
                }
            } else {
                meshNode->instanceIndex = std::make_optional(mesh->instances.size());
                mesh->instances.push_back(newPair);
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
    auto before = std::chrono::high_resolution_clock::now();
    readElementTree(&elementTree->rootNode);
    for (const auto &mesh: meshes) {
        mesh.second->writeGraphicsData();
    }
    auto after = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
    std::cout << "rereadElementTree() in " << duration / 1000.0f << "ms" << std::endl;
}
