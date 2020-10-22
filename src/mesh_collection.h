// mesh_collection.h
// Created by bb1950328 on 03.10.20.
//

#ifndef BRICKSIM_MESH_COLLECTION_H
#define BRICKSIM_MESH_COLLECTION_H

#include "element_tree.h"

class MeshCollection {
public:
    explicit MeshCollection(ElementTree *elementTree);

    std::map<std::pair<void *, bool>, Mesh*> meshes;

    void initializeGraphics();

    void deallocateGraphics();

    void readElementTree(ElementTreeNode *node, const glm::mat4 &parentAbsoluteTransformation);

    void rereadElementTree();

    void drawTriangleGraphics() const;

    void drawLineGraphics() const;

    ElementTreeNode* getElementById(unsigned int id);

private:
    std::vector<ElementTreeNode*> elementsSortedById;
    ElementTree *elementTree;
    std::set<ElementTreeNode*> nodesWithChildrenAlreadyVisited;
    std::map<std::pair<ElementTreeMeshNode*, bool>, size_t> meshInstanceIndices;
};
#endif //BRICKSIM_MESH_COLLECTION_H
