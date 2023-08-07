#pragma once
#include "../element_tree.h"
namespace bricksim::connection {
    class IntersectionGraph {
    public:
        using node_t = std::shared_ptr<etree::MeshNode>;

        std::mutex mtx;
        uomap_t<node_t, uoset_t<node_t>> adjacency;

        void addEdge(const node_t& a, const node_t& b);
        void removeEdge(const node_t& a, const node_t& b);
        void removeNode(const node_t& node);
        void removeAllNodes(const uoset_t<node_t>& nodes);
        std::size_t getNodeCount() const;
        std::size_t getEdgeCount() const;
        const uoset_t<node_t>& getConnected(const node_t& a) const;
        bool hasEdge(const node_t& a, const node_t& b) const;
        void clear();

    protected:
        void removeEdgeHalf(const node_t& a, const node_t& b);
        void removeNodeUnlocked(const node_t& node);
    };
}
