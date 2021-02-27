

#ifndef BRICKSIM_MESH_COLLECTION_H
#define BRICKSIM_MESH_COLLECTION_H

typedef std::pair<void *, bool> mesh_key_t;

#include "element_tree.h"

namespace mesh_collection {
    std::map<mesh_key_t, std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<etree::Node>> elementsSortedById;
    std::shared_ptr<etree::Node> getElementById(unsigned int id);
};

/**
 * the purpose of this class is to manage the meshInstances of a Scene object
 */
class SceneMeshCollection {
private:
    std::map<mesh_key_t, std::shared_ptr<Mesh>> usedMeshes;
    std::set<layer_t> layersInUse;
    scene_id_t scene;
    std::shared_ptr<etree::Node> rootNode;

    std::map<mesh_key_t, std::vector<MeshInstance>> newMeshInstances;
    std::set<std::shared_ptr<etree::Node>> nodesWithChildrenAlreadyVisited;

    void updateMeshInstances();
    void readElementTree(const std::shared_ptr<etree::Node>& node, const glm::mat4 &parentAbsoluteTransformation, std::optional<LdrColorReference> parentColor, std::optional<unsigned int> selectionTargetElementId);
    [[nodiscard]] std::pair<glm::vec3, glm::vec3> getBoundingBoxInternal(std::shared_ptr<const etree::MeshNode> node) const;
public:
    explicit SceneMeshCollection(scene_id_t scene);
    SceneMeshCollection & operator=(SceneMeshCollection&) = delete;
    SceneMeshCollection(const SceneMeshCollection&) = delete;

    void rereadElementTree();
    void updateSelectionContainerBox();

    [[nodiscard]] std::pair<glm::vec3, glm::vec3> getBoundingBox(const std::shared_ptr<const etree::MeshNode>& node) const;
    [[nodiscard]] const std::set<layer_t> &getLayersInUse() const;

    void drawTriangleGraphics(layer_t layer) const;
    void drawLineGraphics(layer_t layer) const;
    void drawOptionalLineGraphics(layer_t layer) const;
};
#endif //BRICKSIM_MESH_COLLECTION_H
