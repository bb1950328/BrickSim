#include "connection_graphviz_generator.h"
#include "../../controller.h"
#include "../../helpers/graphviz_wrapper.h"
#include "../connector_data_provider.h"
#include <spdlog/fmt/fmt.h>

namespace bricksim::connection::visualization {
    //todo add option to export whole thing to a directory and render image for each node (https://stackoverflow.com/a/8077158/8733066)

    const char* getConnectionTypeColor(Connector::Type type1, Connector::Type type2) {
        if (type1 == Connector::Type::CYLINDRICAL && type2 == Connector::Type::CYLINDRICAL) {
            return "blue";
        } else if (type1 == Connector::Type::FINGER || type2 == Connector::Type::FINGER) {
            return "green";
        } else if (type1 == Connector::Type::CLIP || type2 == Connector::Type::CLIP) {
            return "red";
        } else if (type1 == Connector::Type::GENERIC && type2 == Connector::Type::GENERIC) {
            return "orange";
        } else {
            throw std::invalid_argument(magic_enum::enum_name(type1).data());
        }
    }

    GraphVizResult generateGraphviz(const ConnectionGraph& graph, const GraphVizParams& params, const std::shared_ptr<etree::MeshNode>& parentNode) {
        GraphVizResult result;
        if (params.thumbnailDirectory.empty()) {
            result.tmpDirectory = std::filesystem::temp_directory_path() / util::randomAlphanumString(16);
            std::filesystem::create_directory(result.tmpDirectory);
        } else {
            result.tmpDirectory = params.thumbnailDirectory;
        }
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

        const auto bgColor = params.general.darkTheme ? "#333333" : "#ffffff";
        const auto fgColor = params.general.darkTheme ? "#ffffff" : "#000000";

        dot += "graph G {\n";
        dot += fmt::format("\tlayout=\"{}\"\n", magic_enum::enum_name(params.general.layout));
        dot += fmt::format("\tdpi={}\n", params.general.dpi);
        dot += fmt::format("\tbgcolor=\"{}\"\n", bgColor);
        dot += fmt::format("\timagepath=\"{}\"\n", result.tmpDirectory.string());

        for (const auto& [node, adj]: graph.getAdjacencyLists()) {
            if (parentNode != nullptr && !node->isChildOf(parentNode)) {
                continue;
            }
            std::shared_ptr<ldr::File> ldrFile;
            const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(node);
            if (ldrNode != nullptr) {
                ldrFile = ldrNode->ldrFile;
            } else {
                ldrFile = std::dynamic_pointer_cast<etree::ModelInstanceNode>(node)->modelNode->ldrFile;
            }

            const auto nodeId = getNodeId(node);

            std::vector<std::string> labelLines;
            if (params.node.showThumbnail) {
                const auto bgRGB = color::RGB(bgColor);
                graphics::ThumbnailRequest thumbnailRequest = {ldrFile, node->getDisplayColor(), bgRGB};
                const auto filename = thumbnailRequest.getFilename();
                const auto path = result.tmpDirectory / filename;
                if (!std::filesystem::exists(path)) {
                    thumbnailGenerator->getThumbnail(thumbnailRequest)->saveToFile(path);
                }
                labelLines.push_back(fmt::format("<img src=\"{}\" />", filename));
            }
            if (params.node.showTitle) {
                labelLines.push_back(stringutil::escapeXml(ldrFile->metaInfo.title));
            }
            if (params.node.showName) {
                labelLines.push_back(stringutil::escapeXml(ldrFile->metaInfo.name));
            }
            if (params.node.showLocation) {
                const auto location = glm::row(node->getAbsoluteTransformation(), 3);
                labelLines.push_back(fmt::format("[{:g}, {:g}, {:g}]", location.x, location.y, location.z));
            }
            if (labelLines.empty()) {
                labelLines.push_back(std::string(nodeId));
            }
            const auto color = params.node.colorBoxLikePart
                                   ? node->getDisplayColor().get()->value.asHtmlCode()
                                   : fgColor;
            dot += fmt::format("\t{} [label=<<table border=\"0\"><tr><td>{}</td></tr></table>> shape=box tooltip=\"{}\" color=\"{}\"]\n",
                               nodeId,
                               fmt::join(labelLines, "</td></tr><tr><td>"),
                               ldrFile->metaInfo.title,
                               color);
        }

        for (const auto& [node1, adj]: graph.getAdjacencyLists()) {
            if (parentNode != nullptr && !node1->isChildOf(parentNode)) {
                continue;
            }
            const auto id1 = getNodeId(node1);
            for (const auto& [node2, connections]: adj) {
                if (node1.get() < node2.get()) {
                    const auto id2 = getNodeId(node2);
                    if (params.edge.oneLineBetweenNode) {
                        std::vector<connection::DegreesOfFreedom> individualDOFs;
                        std::transform(connections.cbegin(), connections.cend(), std::back_inserter(individualDOFs), [](const auto& conn) {
                            return conn->degreesOfFreedom;
                        });

                        const auto dof = connection::DegreesOfFreedom::reduce(individualDOFs);
                        const auto style = !dof.empty() && params.edge.nonRigidConnectionsDashed ? "dashed" : "solid";
                        const auto* color = connections.size() == 1
                                                ? getConnectionTypeColor(connections[0]->connectorA->type, connections[0]->connectorB->type)
                                                : fgColor;
                        dot += fmt::format("\t{} -- {} [style=\"{}\" color=\"{}\" penwidth=4]\n", id1, id2, style, color);
                    } else {
                        for (const auto& conn: connections) {
                            const auto style = !conn->degreesOfFreedom.empty() && params.edge.nonRigidConnectionsDashed ? "dashed" : "solid";
                            const auto* color = getConnectionTypeColor(conn->connectorA->type, conn->connectorB->type);
                            dot += fmt::format("\t{} -- {} [style=\"{}\" color=\"{}\" penwidth=4]\n", id1, id2, style, color);
                        }
                    }
                }
            }
        }

        dot += "}\n";
        return result;
    }

    GraphVizResult::GraphVizResult() = default;

    GraphVizResult::~GraphVizResult() {
        if (deleteTmpFiles) {
            std::filesystem::remove_all(tmpDirectory);
        }
    }

    bool GraphVizResult::renderToFile(const std::filesystem::path& outFile) const {
        return graphviz_wrapper::renderDot(outFile, dotCode);
    }

    GraphVizResult::GraphVizResult(GraphVizResult&& other) :
        dotCode(other.dotCode), tmpDirectory(other.tmpDirectory), deleteTmpFiles(other.deleteTmpFiles) {
        other.deleteTmpFiles = false;
    }

    GraphVizResult& GraphVizResult::operator=(GraphVizResult&& other) {
        dotCode = other.dotCode;
        tmpDirectory = other.tmpDirectory;
        deleteTmpFiles = other.deleteTmpFiles;
        other.deleteTmpFiles = false;
        return *this;
    }

    GraphVizResult::GraphVizResult(const GraphVizResult& other) :
        dotCode(other.dotCode), tmpDirectory(other.tmpDirectory), deleteTmpFiles(false) {}

    GraphVizResult& GraphVizResult::operator=(const GraphVizResult& other) {
        dotCode = other.dotCode;
        tmpDirectory = other.tmpDirectory;
        deleteTmpFiles = false;
        return *this;
    }
}
