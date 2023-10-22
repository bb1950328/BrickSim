#pragma once

#include "../connection.h"
#include "../connection_graph.h"
#include <string>

namespace bricksim::connection::visualization {
    enum class LayoutEngine {
        dot,
        neato,
        fdp,
        sfdp,
        circo,
        twopi,
        osage,
        patchwork,
    };
    struct GraphVizParams {
        std::filesystem::path thumbnailDirectory;
        bool showPartThumbnails = true;
        LayoutEngine layout = LayoutEngine::dot;
    };

    class GraphVizResult {
    public:
        std::string dotCode;
        std::filesystem::path tmpDirectory;
        bool deleteTmpFiles = true;

        GraphVizResult();
        virtual ~GraphVizResult();
        void renderToFile(const std::filesystem::path& outFile) const;
    };
    GraphVizResult generateGraphviz(const ConnectionGraph& graph, const GraphVizParams& params, const std::shared_ptr<etree::MeshNode>& parentNode);
}
