#include "connection_graphviz_generator.h"
#include "../../controller.h"
#include "../../helpers/graphviz_wrapper.h"
#include <spdlog/fmt/fmt.h>

namespace bricksim::connection::visualization {
    //todo add option to export whole thing to a directory and render image for each node (https://stackoverflow.com/a/8077158/8733066)

    GraphVizResult generateGraphviz(const ConnectionGraph& graph) {
        GraphVizResult result;
        auto& dot = result.dotCode;

        const auto thumbnailGenerator = controller::getThumbnailGenerator();

        uomap_t<ConnectionGraph::node_t, std::string> nodeIds;
        uint64_t nextNodeId = 0;
        const auto getNodeId = [&nodeIds, &nextNodeId](const ConnectionGraph::node_t& node) {
            const auto it = nodeIds.find(node);
            if (it != nodeIds.end()) {
                return std::string_view(it->second);
            } else {
                return std::string_view(nodeIds.emplace(node, std::to_string(nextNodeId++)).first->second);
            }
        };

        dot += "graph G {\n";
        for (const auto& [node, adj]: graph.adjacencyLists) {
            const auto thumbnailPath = result.tmpDirectory / fmt::format("{}_{}.png", node->ldrFile->metaInfo.name, node->getDisplayColor().code);
            if (!std::filesystem::exists(thumbnailPath)) {
                thumbnailGenerator->getThumbnail(node->ldrFile, node->getDisplayColor())->saveToFile(thumbnailPath);
            }
            dot += fmt::format("\t{} [label=\"{}\" image=\"{}\" shape=box imagepos=tc labelloc=b]\n", getNodeId(node), node->ldrFile->metaInfo.title, thumbnailPath.string());
        }

        for (const auto& [node1, adj]: graph.adjacencyLists) {
            for (const auto& [node2, connections]: adj) {
                if (node1.get() < node2.get()) {
                    for (const auto& conn: connections) {
                        dot += fmt::format("\t{} -- {}\n", getNodeId(node1), getNodeId(node2));
                    }
                }
            }
        }

        dot += "}\n";
        return result;
    }
    GraphVizResult::GraphVizResult() :
        dotCode(),
        tmpDirectory(std::filesystem::temp_directory_path() / util::randomAlphanumString(16)) {
        std::filesystem::create_directory(tmpDirectory);
    }
    GraphVizResult::~GraphVizResult() {
        std::filesystem::remove_all(tmpDirectory);
    }
    void GraphVizResult::renderToFile(const std::filesystem::path& outFile) const {
        graphviz_wrapper::renderDot(outFile, dotCode);
    }
}
