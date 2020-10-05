// mesh_collection.cpp
// Created by bab21 on 03.10.20.
//

#include "mesh_collection.h"
#include "statistic.h"

void MeshCollection::initializeGraphics() {
    for (const auto &pair: meshes) {
        pair.second->initializeGraphics();
    }
}

void MeshCollection::drawGraphics(Shader *triangleShader, Shader *lineShader) {
    triangleShader->use();
    for (const auto &pair: meshes) {
        pair.second->drawTriangleGraphics(triangleShader);
    }
    lineShader->use();
    for (const auto &pair: meshes) {
        pair.second->drawLineGraphics(lineShader);
    }
}

void MeshCollection::deallocateGraphics() {
    for (const auto &pair: meshes) {
        pair.second->deallocateGraphics();
    }
}

void MeshCollection::readElementTree(ElementTreeNode *node) {
    if (node->getType()==ET_TYPE_LDRFILE) {
        auto *ldrNode = dynamic_cast<ElementTreeLdrNode *>(node);
        auto it = meshes.find(ldrNode->ldrFile);
        Mesh *mesh;
        if (ldrNode->ldrFile->type==PART) {
            stats::Counters::totalBrickCount++;
        }
        if (it != meshes.end()) {
            mesh = it->second;
        } else {
            mesh = new Mesh();
            meshes[ldrNode->ldrFile] = mesh;
            mesh->name = ldrNode->ldrFile->getDescription();
            if (ldrNode->ldrFile->type==PART) {
                stats::Counters::individualBrickCount++;
            }
            ldrNode->addToMesh(mesh);
        }
        mesh->instances.emplace_back(ldrNode->ldrColor, node->getAbsoluteTransformation());
    }
    for (const auto &child: node->children) {
        readElementTree(child);
    }
}

MeshCollection::MeshCollection(ElementTree *elementTree) {
    this->elementTree = elementTree;
}

void MeshCollection::readElementTree() {
    readElementTree(&elementTree->rootNode);
}
