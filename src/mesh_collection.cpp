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
        if (node->getType() == ET_TYPE_LDRFILE) {
            auto *ldrNode = dynamic_cast<ElementTreeLdrNode *>(node);
            auto it = meshes.find(ldrNode->ldrFile);
            Mesh *mesh;
            if (ldrNode->ldrFile->metaInfo.type == PART) {
                stats::Counters::totalBrickCount++;
            }
            if (it != meshes.end()) {
                mesh = it->second;
            } else {
                mesh = new Mesh();
                meshes[ldrNode->ldrFile] = mesh;
                mesh->name = ldrNode->ldrFile->getDescription();
                if (ldrNode->ldrFile->metaInfo.type == PART) {
                    stats::Counters::individualBrickCount++;
                }
                ldrNode->addToMesh(mesh);
            }
            mesh->instances.emplace_back(ldrNode->ldrColor, node->getAbsoluteTransformation());
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
    for (const auto& mesh: meshes) {
        mesh.second->instances.clear();
    }
    readElementTree(&elementTree->rootNode);
    for (const auto& mesh: meshes) {
        mesh.second->writeGraphicsData();
    }
    auto after = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
    std::cout << "rereadElementTree() in " << duration/1000.0f << "ms" << std::endl;
}
