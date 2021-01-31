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

    void rereadElementTree();

    void drawTriangleGraphics(layer_t layer) const;
    void drawLineGraphics(layer_t layer) const;
    void drawOptionalLineGraphics(layer_t layer) const;

    etree::Node* getElementById(unsigned int id);

    void updateSelectionContainerBox();

    std::pair<glm::vec3, glm::vec3> getBoundingBox(const etree::MeshNode *node) const;
    [[nodiscard]] const std::set<layer_t> &getLayersInUse() const;

    virtual ~MeshCollection();
private:
    std::vector<etree::Node*> elementsSortedById;

    etree::ElementTree *elementTree;
    std::set<etree::Node*> nodesWithChildrenAlreadyVisited;

    std::map<std::pair<void *, bool>, std::vector<MeshInstance>> newMeshInstances;
    std::set<layer_t> layersInUse;
    void updateMeshInstances();
    void readElementTree(etree::Node *node, const glm::mat4 &parentAbsoluteTransformation, LdrColor *parentColor, std::optional<unsigned int> selectionTargetElementId);
    std::pair<glm::vec3, glm::vec3> getBoundingBoxInternal(const etree::MeshNode *node) const;
};
#endif //BRICKSIM_MESH_COLLECTION_H
