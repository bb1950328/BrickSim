// mesh_collection.h
// Created by bb1950328 on 03.10.20.
//

#ifndef BRICKSIM_MESH_COLLECTION_H
#define BRICKSIM_MESH_COLLECTION_H

#include "element_tree.h"

class MeshCollection {
public:
    MeshCollection(ElementTree *elementTree);

    std::map<LdrFile *, Mesh*> meshes;

    void initializeGraphics();

    void deallocateGraphics();

    void readElementTree(ElementTreeNode *node);

    void rereadElementTree();

    void drawTriangleGraphics(const Shader *triangleShader) const;

    void drawLineGraphics(const Shader *lineShader) const;

private:

    ElementTree *elementTree;

};
#endif //BRICKSIM_MESH_COLLECTION_H
