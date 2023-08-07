#include "intersection_graph.h"
namespace bricksim::connection {

    void IntersectionGraph::addEdge(const IntersectionGraph::node_t& a, const IntersectionGraph::node_t& b) {
        std::lock_guard lg(mtx);
        adjacency[a].insert(b);
        adjacency[b].insert(a);
    }
    void IntersectionGraph::removeEdge(const IntersectionGraph::node_t& a, const IntersectionGraph::node_t& b) {
        std::lock_guard lg(mtx);
        removeEdgeHalf(a, b);
        removeEdgeHalf(b, a);
    }
    const uoset_t<IntersectionGraph::node_t>& IntersectionGraph::getConnected(const IntersectionGraph::node_t& a) const {
        auto it = adjacency.find(a);
        if (it != adjacency.end()) {
            return it->second;
        } else {
            const static uoset_t<node_t> empty;
            return empty;
        }
    }
    void IntersectionGraph::removeEdgeHalf(const IntersectionGraph::node_t& a, const IntersectionGraph::node_t& b) {
        auto it = adjacency.find(a);
        if (it != adjacency.end()) {
            it->second.erase(b);
            if (it->second.empty()) {
                adjacency.erase(it);
            }
        }
    }
    void IntersectionGraph::clear() {
        std::lock_guard lg(mtx);
        adjacency.clear();
    }
    void IntersectionGraph::removeNode(const IntersectionGraph::node_t& node) {
        std::lock_guard lg(mtx);
        removeNodeUnlocked(node);
    }
    void IntersectionGraph::removeNodeUnlocked(const IntersectionGraph::node_t& node) {
        auto it = adjacency.find(node);
        if (it != adjacency.end()) {
            for (const auto& neighbor: it->second) {
                removeEdgeHalf(neighbor, node);
            }
            adjacency.erase(it);
        }
    }
    void IntersectionGraph::removeAllNodes(const uoset_t<IntersectionGraph::node_t>& nodes) {
        std::lock_guard lg(mtx);
        for (const auto& item: nodes) {
            removeNodeUnlocked(item);
        }
    }
    std::size_t IntersectionGraph::getNodeCount() const {
        return adjacency.size();
    }
    std::size_t IntersectionGraph::getEdgeCount() const {
        std::size_t count = 0;
        for (const auto& item: adjacency) {
            count += item.second.size();
        }
        return count / 2;
    }
    bool IntersectionGraph::hasEdge(const IntersectionGraph::node_t& a, const IntersectionGraph::node_t& b) const {
        const auto it = adjacency.find(a);
        return it != adjacency.end() && it->second.contains(b);
    }
}
