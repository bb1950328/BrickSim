#include "connection_graphviz_generator.h"
#include <spdlog/fmt/fmt.h>

namespace bricksim::connection::visualization {
    //todo add option to export whole thing to a directory and render image for each node (https://stackoverflow.com/a/8077158/8733066)

    // todo add option to call graphviz automatically and open file dialog to set the output file

    std::string generateGraphviz(const ConnectionGraph& graph) {
        std::string result;

        uomap_t<ConnectionGraph::node_t, std::string> nodeIds;
        uint64_t nextNodeId = 0;
        const auto getNodeId = [&nodeIds, &nextNodeId](const ConnectionGraph::node_t& node){
            const auto it = nodeIds.find(node);
            if (it != nodeIds.end()) {
                return std::string_view(it->second);
            } else {
                return std::string_view(nodeIds.emplace(node, std::to_string(nextNodeId++)).first->second);
            }
        };

        result += "graph G {\n";
        for (const auto& [node, adj]: graph.adjacencyLists) {
            result += fmt::format("\t{} [label=\"{}\" shape=box]\n", getNodeId(node), node->ldrFile->metaInfo.title);
        }

        for (const auto& [node1, adj]: graph.adjacencyLists) {
            for (const auto& [node2, connections]: adj) {
                if (node1.get() < node2.get()) {
                    for (const auto& conn: connections) {
                        result += fmt::format("\t{} -- {}\n", getNodeId(node1), getNodeId(node2));
                    }
                }
            }
        }

        result += "}\n";
        return result;
    }
}
