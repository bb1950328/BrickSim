// mesh_collection.h
// Created by bab21 on 03.10.20.
//

#ifndef BRICKSIM_MESH_COLLECTION_H
#define BRICKSIM_MESH_COLLECTION_H

#include "element_tree.h"

class MeshCollection {
public:
    MeshCollection(ElementTree *elementTree);

    std::map<LdrFile *, Mesh*> meshes;

    void initializeGraphics();

    void drawGraphics(Shader *triangleShader);

    void deallocateGraphics();

    void readElementTree(ElementTreeNode *node);

    void readElementTree();

private:

    ElementTree *elementTree;
};
#endif //BRICKSIM_MESH_COLLECTION_H
