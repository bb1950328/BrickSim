#pragma once

#include "../connection.h"
#include "../connection_graph.h"
#include <string>

namespace bricksim::connection::visualization {
    class GraphVizResult {
    public:
        std::string dotCode;
        std::filesystem::path tmpDirectory;
        bool deleteTmpFiles = true;

        GraphVizResult();
        virtual ~GraphVizResult();
        void renderToFile(const std::filesystem::path& outFile) const;
    };
    GraphVizResult generateGraphviz(const ConnectionGraph& graph);
}
