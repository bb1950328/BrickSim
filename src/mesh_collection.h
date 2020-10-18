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

    void readElementTree(ElementTreeNode *node);

    void rereadElementTree();

    void drawTriangleGraphics() const;

    void drawLineGraphics() const;

private:

    ElementTree *elementTree;
    unsigned int currentElementId;
};
#endif //BRICKSIM_MESH_COLLECTION_H
