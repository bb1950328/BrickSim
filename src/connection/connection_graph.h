#pragma once

#include "../element_tree.h"
#include "connection.h"

namespace bricksim::connection {
    class ConnectionGraph {
        //todo extract undirected cyclic multigraph base class with two template params
    public:
        using node_t = std::shared_ptr<etree::MeshNode>;
        using edge_t = std::shared_ptr<Connection>;
        using adjacency_list_t = uomap_t<node_t, uomap_t<node_t, std::vector<edge_t>>>;

        void addConnection(const node_t& a, const node_t& b, const edge_t& edge);

        void removeConnection(const node_t& a, const node_t& b, const edge_t& edge);

        void removeAllConnections(const node_t& a, const node_t& b);
        void removeAllConnections(const node_t& a);
        void removeAllConnections(const uoset_t<node_t>& toRemove);

        [[nodiscard]] const adjacency_list_t& getAdjacencyLists() const;
        [[nodiscard]] const std::vector<edge_t>& getConnections(const node_t& a, const node_t& b) const;
        [[nodiscard]] const uomap_t<node_t, std::vector<edge_t>>& getConnections(const node_t& node) const;

        [[nodiscard]] std::size_t countTotalConnections() const;
        [[nodiscard]] std::vector<uoset_t<node_t>> findAllCliques() const;

    protected:
        adjacency_list_t adjacencyLists;
        std::mutex lock;

        void findRestOfClique(uoset_t<node_t>& nodes, const ConnectionGraph::node_t& current) const;

    private:
        /**
         * Not thread-safe on its own, make sure that `adjacencyLists` is locked!
         */
        std::array<std::reference_wrapper<std::vector<ConnectionGraph::edge_t>>, 2> getBothVectors(const node_t& a, const node_t& b);
    };
}
