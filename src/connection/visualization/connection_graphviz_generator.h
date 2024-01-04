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
        struct NodeParams {
            bool showThumbnail = true;
            bool showLocation = false;
            bool showName = false;
            bool showTitle = true;
            bool colorBoxLikePart = true;
        };

        struct EdgeParams {
            bool colorLineByConnectorType = true;
            bool nonRigidConnectionsDashed = true;
            bool oneLineBetweenNode = false;
        };

        struct GeneralParams {
            LayoutEngine layout = LayoutEngine::dot;
            int dpi = 96;
            bool darkTheme = true;
        };

        std::filesystem::path thumbnailDirectory;
        GeneralParams general;
        NodeParams node;
        EdgeParams edge;
    };

    class GraphVizResult {
    public:
        std::string dotCode;
        std::filesystem::path tmpDirectory;
        bool deleteTmpFiles = true;

        GraphVizResult();
        GraphVizResult(GraphVizResult&& other);
        GraphVizResult(const GraphVizResult& other);
        GraphVizResult& operator=(GraphVizResult&& other);
        GraphVizResult& operator=(const GraphVizResult& other);
        virtual ~GraphVizResult();
        bool renderToFile(const std::filesystem::path& outFile) const;
    };

    GraphVizResult generateGraphviz(const ConnectionGraph& graph, const GraphVizParams& params, const std::shared_ptr<etree::MeshNode>& parentNode);
}
