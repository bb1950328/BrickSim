#pragma once

#include "../graphics/mesh/mesh_collection.h"
#include "data.h"
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

        Editor& editor;
        fcl::DynamicAABBTreeCollisionManagerf manager;
        uomap_t<std::shared_ptr<etree::Node>, NodeData> nodeData;
        ConnectionGraph graph;
        uoset_t<std::shared_ptr<etree::LdrNode>> outdatedInGraph;

        void updateCollisionData();
        void updateGraph();

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
        fcl::Transform3f getCollisionBoxTransform(const std::shared_ptr<etree::LdrNode>& node) const;

    public:
        explicit Engine(Editor& editor);

        void update();

        const ConnectionGraph& getGraph() const;
        bool fclCallback(fcl::CollisionObjectf* o0, fcl::CollisionObjectf* o1);
    };
}
