#include "connection_graph.h"

namespace bricksim::connection {
    void ConnectionGraph::addConnection(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b, const ConnectionGraph::edge_t& edge) {
        std::lock_guard<std::mutex> lg(lock);
        for (auto& vec: getBothVectors(a, b)) {
            vec.get().push_back(edge);
        }
    }

    void ConnectionGraph::removeAllConnections(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) {
        std::lock_guard<std::mutex> lg(lock);
        adjacencyLists[a].erase(b);
        adjacencyLists[b].erase(a);
    }

    void ConnectionGraph::removeAllConnections(const ConnectionGraph::node_t& a) {
        std::lock_guard<std::mutex> lg(lock);
        adjacencyLists.erase(a);
        for (auto& [x, adj]: adjacencyLists) {
            adj.erase(a);
        }
    }

    void ConnectionGraph::removeAllConnections(const uoset_t<ConnectionGraph::node_t>& toRemove) {
        std::lock_guard<std::mutex> lg(lock);
        if (toRemove.empty()) {
            return;
        }
        for (const auto& a: toRemove) {
            adjacencyLists.erase(a);
        }
        for (auto& [x, adj]: adjacencyLists) {
            for (const auto& a: toRemove) {
                adj.erase(a);
            }
        }
    }

    const std::vector<ConnectionGraph::edge_t>& ConnectionGraph::getConnections(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) const {
        const auto it = adjacencyLists.find(a);
        if (it != adjacencyLists.end()) {
            const auto it2 = it->second.find(b);
            if (it2 != it->second.end()) {
                return it2->second;
            }
        }
        const static std::vector<ConnectionGraph::edge_t> empty;
        return empty;
    }

    void ConnectionGraph::removeConnection(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b, const ConnectionGraph::edge_t& edge) {
        std::lock_guard<std::mutex> lg(lock);
        for (auto& vec_wrapper: getBothVectors(a, b)) {
            auto& vec = vec_wrapper.get();
            const auto it = std::find(vec.begin(), vec.end(), edge);
            if (it != vec.end()) {
                vec.erase(it);
            }
        }
    }

    std::array<std::reference_wrapper<std::vector<ConnectionGraph::edge_t>>, 2> ConnectionGraph::getBothVectors(const ConnectionGraph::node_t& a, const ConnectionGraph::node_t& b) {
        return {adjacencyLists[a][b], adjacencyLists[b][a]};
    }

    const uomap_t<ConnectionGraph::node_t, std::vector<ConnectionGraph::edge_t>>& ConnectionGraph::getConnections(const ConnectionGraph::node_t& node) const {
        const auto it = adjacencyLists.find(node);
        if (it == adjacencyLists.end()) {
            const static uomap_t<node_t, std::vector<edge_t>> empty;
            return empty;
        }
        return it->second;
    }

    std::size_t ConnectionGraph::countTotalConnections() const {
        std::size_t total = 0;
        for (const auto& i: adjacencyLists) {
            for (const auto& j: i.second) {
                total += j.second.size();
            }
        }
        return total / 2;
    }

    const ConnectionGraph::adjacency_list_t& ConnectionGraph::getAdjacencyLists() const {
        return adjacencyLists;
    }

    void ConnectionGraph::findRestOfClique(uoset_t<ConnectionGraph::node_t>& nodes, const ConnectionGraph::node_t& current) const {
        nodes.insert(current);

        const auto& currentNeighbors = adjacencyLists.find(current)->second;
        for (const auto& n: currentNeighbors) {
            if (!n.second.empty() && !nodes.contains(n.first)) {
                findRestOfClique(nodes, n.first);
            }
        }
    }

    std::vector<uoset_t<ConnectionGraph::node_t>> ConnectionGraph::findAllCliques() const {
        std::vector<uoset_t<node_t>> result;
        uoset_t<node_t> unprocessed;
        unprocessed.reserve(adjacencyLists.size());
        std::transform(adjacencyLists.cbegin(), adjacencyLists.cend(),
                       std::inserter(unprocessed, unprocessed.end()),
                       [](auto entry) { return entry.first; });

        while (!unprocessed.empty()) {
            const auto n = *unprocessed.begin();
            unprocessed.erase(n);
            uoset_t<node_t> clique;
            findRestOfClique(clique, n);
            for (const auto& c: clique) {
                unprocessed.erase(c);
            }
            result.push_back(clique);
        }

        return result;
    }
}
