#pragma once

#include "../../element_tree.h"
#include "mesh.h"
#include "../../helpers/util.h"
#include <set>

namespace bricksim::mesh {
    struct mesh_key_t {
        mesh_identifier_t meshIdentifier;
        bool windingInversed;
        size_t texmapHash;
        bool operator==(const mesh_key_t& rhs) const;
        bool operator!=(const mesh_key_t& rhs) const;
    };
}

namespace std {
    template<>
    struct hash<bricksim::mesh::mesh_key_t> {
        std::size_t operator()(bricksim::mesh::mesh_key_t value) const {
            return bricksim::util::combinedHash(value.meshIdentifier, value.windingInversed, value.texmapHash);
        }
    };
}

namespace bricksim::mesh {
    /**
 * the purpose of this class is to manage the meshInstances of a Scene object
 */
    class SceneMeshCollection {
    private:
        uoset_t<std::shared_ptr<Mesh>> usedMeshes, lastUsedMeshes;
        oset_t<layer_t> layersInUse;
        scene_id_t scene;
        std::shared_ptr<etree::Node> rootNode;

        uomap_t<mesh_key_t, std::vector<MeshInstance>> newMeshInstances;
        uoset_t<std::shared_ptr<etree::Node>> nodesWithChildrenAlreadyVisited;
        std::vector<std::shared_ptr<etree::Node>> elementsSortedById;

        uint64_t lastElementTreeReadVersion = 0;

        void updateMeshInstances();
        void readElementTree(const std::shared_ptr<etree::Node>& node,
                             const glm::mat4& parentAbsoluteTransformation,
                             std::optional<ldr::ColorReference> parentColor,
                             std::optional<unsigned int> selectionTargetElementId,
                             const std::shared_ptr<ldr::TexmapStartCommand>& parentTexmap);

        static uomap_t<mesh_key_t, std::shared_ptr<Mesh>> allMeshes;

    public:
        explicit SceneMeshCollection(scene_id_t scene);
        SceneMeshCollection& operator=(SceneMeshCollection&) = delete;
        SceneMeshCollection(const SceneMeshCollection&) = delete;

        void rereadElementTreeIfNeeded();
        //void updateSelectionContainerBoxIfNeeded();

        [[nodiscard]] AxisAlignedBoundingBox getAbsoluteAABB(const std::shared_ptr<const etree::MeshNode>& node) const;
        [[nodiscard]] AxisAlignedBoundingBox getRelativeAABB(const std::shared_ptr<const etree::MeshNode>& node) const;
        [[nodiscard]] std::optional<RotatedBoundingBox> getRotatedBBox(const std::shared_ptr<const etree::MeshNode>& node) const;
        [[nodiscard]] const oset_t<layer_t>& getLayersInUse() const;
        [[nodiscard]] std::shared_ptr<etree::Node> getElementById(element_id_t id) const;
        [[nodiscard]] const std::shared_ptr<etree::Node>& getRootNode() const;
        void setRootNode(const std::shared_ptr<etree::Node>& newRootNode);

        void drawTriangleGraphics(layer_t layer) const;
        void drawTexturedTriangleGraphics(layer_t layer) const;
        void drawLineGraphics(layer_t layer) const;
        void drawOptionalLineGraphics(layer_t layer) const;

        static mesh_key_t getMeshKey(const std::shared_ptr<etree::MeshNode>& node, bool windingOrderInverse, const std::shared_ptr<ldr::TexmapStartCommand>& texmap);
        static std::shared_ptr<Mesh> getMesh(mesh_key_t key, const std::shared_ptr<etree::MeshNode>& node, const std::shared_ptr<ldr::TexmapStartCommand>& texmap);
        [[nodiscard]] const uoset_t<std::shared_ptr<Mesh>>& getUsedMeshes() const;
        static void deleteAllMeshes();
    };
}
