// mesh_collection.h
// Created by bb1950328 on 03.10.20.
//

#ifndef BRICKSIM_MESH_COLLECTION_H
#define BRICKSIM_MESH_COLLECTION_H

#include "element_tree.h"

class MeshCollection {
public:
    explicit MeshCollection(etree::ElementTree *elementTree);

    std::map<std::pair<void *, bool>, Mesh*> meshes;

    void initializeGraphics();

    void deallocateGraphics();

    void readElementTree(etree::Node *node, const glm::mat4 &parentAbsoluteTransformation,
                         LdrColor *parentColor);

    void rereadElementTree();

    void drawTriangleGraphics() const;
    void drawLineGraphics() const;
    void drawOptionalLineGraphics() const;

    etree::Node* getElementById(unsigned int id);

private:
    std::vector<etree::Node*> elementsSortedById;
    etree::ElementTree *elementTree;
    std::set<etree::Node*> nodesWithChildrenAlreadyVisited;
    std::map<std::pair<void *, bool>, std::vector<MeshInstance>> newMeshInstances;

    void updateMeshInstances();
};
#endif //BRICKSIM_MESH_COLLECTION_H
