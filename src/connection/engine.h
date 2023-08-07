#pragma once

#include "../graphics/mesh/mesh_collection.h"
#include "connection.h"
#include "connection_graph.h"
#include "fcl/broadphase/broadphase_dynamic_AABB_tree.h"

namespace bricksim {
    class Editor;
}
namespace bricksim::connection {

    class Engine {
    private:
        struct NodeData {
            etree::Node::version_t lastUpdatedVersion;
            etree::Node::version_t lastUpdatedSelfVersion;
            std::unique_ptr<fcl::CollisionObjectf> collisionObj;
            uoset_t<std::shared_ptr<etree::Node>> children;
        };
        using broadphase_collision_pair_t = std::array<fcl::CollisionObjectf*, 2>;

        Editor& editor;
        fcl::DynamicAABBTreeCollisionManagerf manager;
        uomap_t<std::shared_ptr<etree::Node>, NodeData> nodeData;
        ConnectionGraph graph;
        uoset_t<ConnectionGraph::node_t> outdatedInGraph;
        std::shared_ptr<etree::Node> lastEditingModel;

        static constexpr bool partNodeCollsionOnly = false;

        void updateCollisionData(float* progress, float progressMultiplicator);
        void updateGraph(float* progress, float progressStart);

        void handleBroadphaseCollision(const broadphase_collision_pair_t& item);

        void resetData();

        //todo try to improve manager.registerObject, manager.update and manager.unregisterObject calls
        // so that the binary tree is only balanced once (check if performance is better)
        /**
         * updates @param node in the manager recursively
         */
        void updateNodeData(const std::shared_ptr<etree::Node>& node);
        /**
         * updates @param node in the manager recursively
         * @param it is the iterator to the corresponding element in @a nodeData
         */
        void updateNodeData(const std::shared_ptr<etree::Node>& node, decltype(nodeData)::iterator it);
        /**
         * removes data of @param node from the manager
         */
        void removeNodeData(const std::shared_ptr<etree::Node>& node);

        const std::shared_ptr<etree::Node>& convertRawNodePtr(void* rawPtr) const;

    public:
        explicit Engine(Editor& editor);

        void update(float* progress);
        void update();

        const ConnectionGraph& getGraph() const;
    };
}
