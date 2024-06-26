#pragma once

#include "../graphics/mesh/mesh_collection.h"
#include "../graphics/scene.h"
#include "connection.h"
#include "connection_graph.h"
#include "fcl/broadphase/broadphase_dynamic_AABB_tree.h"
#include "fcl/narrowphase/collision_object.h"
#include "intersection_graph.h"

namespace bricksim {
    class Editor;
}

namespace bricksim::connection {
    using broadphase_collision_pair_t = std::array<fcl::CollisionObjectf*, 2>;

    class Engine {
    private:
        struct NodeData {
            etree::Node::version_t lastUpdatedVersion;
            etree::Node::version_t lastUpdatedSelfVersion;
            std::unique_ptr<fcl::CollisionObjectf> collisionObj;
            uoset_t<std::shared_ptr<etree::Node>> children;
        };

        std::weak_ptr<graphics::Scene> scene;
        fcl::DynamicAABBTreeCollisionManagerf manager;
        uomap_t<std::shared_ptr<etree::Node>, NodeData> nodeData;
        IntersectionGraph intersections;
        ConnectionGraph connections;
        uoset_t<ConnectionGraph::node_t> outdatedInGraphs;

        static constexpr bool partNodeCollsionOnly = false;

        void updateCollisionData(const std::shared_ptr<etree::Node>& rootNode, float* progress, float progressMultiplicator);
        void updateGraph(float* progress, float progressStart);

        void handleNewIntersection(const broadphase_collision_pair_t& item);

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
        Engine();

        void setScene(const std::weak_ptr<graphics::Scene>& scene);

        void update(const std::shared_ptr<etree::Node>& rootNode, float* progress);
        void update(const std::shared_ptr<etree::Node>& rootNode);

        std::vector<std::pair<float, std::shared_ptr<etree::MeshNode>>> getNodesNearRay(Ray3 ray, float distanceLimit, float radiusLimit);

        const IntersectionGraph& getIntersections() const;
        const ConnectionGraph& getConnections() const;

        friend bool updateCallback(fcl::CollisionObjectf* o0, fcl::CollisionObjectf* o1, void* cdata);
        friend bool rayIntersectionCallback(fcl::CollisionObjectf* o0, fcl::CollisionObjectf* o1, void* cdata);
    };
}
