

#ifndef BRICKSIM_MESH_COLLECTION_H
#define BRICKSIM_MESH_COLLECTION_H

typedef std::pair<void *, bool> mesh_key_t;

#include "element_tree.h"

class MeshCollection {
public:
    explicit MeshCollection(std::shared_ptr<etree::RootNode> elementTree);
    MeshCollection(const MeshCollection&) = delete;
    MeshCollection& operator=(MeshCollection&) = delete;

    std::map<mesh_key_t, std::shared_ptr<Mesh>> meshes;

    void initializeGraphics();

    void rereadElementTree();

    void drawTriangleGraphics(layer_t layer) const;
    void drawLineGraphics(layer_t layer) const;
    void drawOptionalLineGraphics(layer_t layer) const;

    std::shared_ptr<etree::Node> getElementById(unsigned int id);

    void updateSelectionContainerBox();

    [[nodiscard]] std::pair<glm::vec3, glm::vec3> getBoundingBox(const std::shared_ptr<const etree::MeshNode>& node) const;
    [[nodiscard]] const std::set<layer_t> &getLayersInUse() const;
private:
    std::vector<std::shared_ptr<etree::Node>> elementsSortedById;

    std::shared_ptr<etree::RootNode> elementTree;
    std::set<std::shared_ptr<etree::Node>> nodesWithChildrenAlreadyVisited;

    std::map<mesh_key_t, std::vector<MeshInstance>> newMeshInstances;
    std::set<layer_t> layersInUse;
    void updateMeshInstances();
    void readElementTree(const std::shared_ptr<etree::Node>& node, const glm::mat4 &parentAbsoluteTransformation, std::optional<LdrColorReference> parentColor, std::optional<unsigned int> selectionTargetElementId);
    [[nodiscard]] std::pair<glm::vec3, glm::vec3> getBoundingBoxInternal(std::shared_ptr<const etree::MeshNode> node) const;
};
#endif //BRICKSIM_MESH_COLLECTION_H
