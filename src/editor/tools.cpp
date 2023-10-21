#include "tools.h"
#include <spdlog/fmt/bundled/format.h>

namespace bricksim::tools {
    namespace {
        struct SelectHandler : NodeClickHandler {
            void operator()(const std::shared_ptr<Editor>& editor, const std::shared_ptr<etree::Node>& node, bool ctrl, bool shift) override {
                if (node) {
                    if (ctrl) {
                        editor->nodeSelectAddRemove(node);
                    } else if (shift) {
                        editor->nodeSelectUntil(node);
                    } else {
                        editor->nodeSelectSet(node);
                    }
                } else {
                    editor->nodeSelectNone();
                }
            }
        };

        struct SelectConnectedHandler : NodeClickHandler {
            const bool strongOnly;
            explicit SelectConnectedHandler(const bool strongOnly) :
                strongOnly(strongOnly) {
            }
            void operator()(const std::shared_ptr<Editor>& editor, const std::shared_ptr<etree::Node>& node, bool ctrl, bool shift) override {
                if (node == nullptr) {
                    editor->nodeSelectNone();
                    return;
                }
                const auto meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node);
                if (meshNode == nullptr) {
                    return;
                }
                uoset_t<std::shared_ptr<etree::Node>> selected = {node};

                auto& engine = editor->getConnectionEngine();
                engine.update(editor->getEditingModel());
                addConnectionsOfNode(engine.getConnections(), selected, meshNode, shift);

                editor->nodeSelectSet(selected);
            }
            void addConnectionsOfNode(const connection::ConnectionGraph& graph, uoset_t<std::shared_ptr<etree::Node>>& selected, const std::shared_ptr<etree::MeshNode>& node, bool recursive) {
                for (const auto& co: graph.getConnections(node)) {
                    if (strongOnly) {
                        std::vector<connection::DegreesOfFreedom> dofs;
                        std::transform(co.second.begin(), co.second.end(), std::back_inserter(dofs), [](const std::shared_ptr<connection::Connection>& conn) {
                            return conn->degreesOfFreedom;
                        });
                        const auto nodeDOF = connection::DegreesOfFreedom::reduce(dofs);
                        if (!nodeDOF.empty()) {
                            return;
                        }
                    }
                    if (selected.insert(co.first).second && recursive) {
                        addConnectionsOfNode(graph, selected, co.first, recursive);
                    }
                }
            }
        };

        using namespace std::string_view_literals;
        const ToolData SELECT = {
                Tool::SELECT,
                "Select"sv,
                gui::icons::IconType::Select,
                std::make_unique<SelectHandler>(),
        };

        const ToolData SELECT_CONNECTED = {
                Tool::SELECT_CONNECTED,
                "Select Connected"sv,
                gui::icons::IconType::SelectConnected,
                std::make_unique<SelectConnectedHandler>(false),
        };

        const ToolData SELECT_STRONGLY_CONNECTED = {
                Tool::SELECT_STRONGLY_CONNECTED,
                "Select Strongly Connected"sv,
                gui::icons::IconType::SelectStronglyConnected,
                std::make_unique<SelectConnectedHandler>(true),
        };

        Tool activeTool = Tool::SELECT;
    }

    ToolData::ToolData(Tool tool,
                       const std::string_view& name,
                       const gui::icons::IconType icon,
                       std::unique_ptr<NodeClickHandler> handleNodeClicked) :
        tool(tool),
        name(name),
        icon(icon),
        nameWithIcon(fmt::format("{}{}", gui::icons::getGlyph(icon, gui::icons::Icon36), name)),
        handleNodeClicked(std::move(handleNodeClicked)) {
    }

    const ToolData& getData(Tool tool) {
        switch (tool) {
            case Tool::SELECT: return SELECT;
            case Tool::SELECT_CONNECTED: return SELECT_CONNECTED;
            case Tool::SELECT_STRONGLY_CONNECTED: return SELECT_STRONGLY_CONNECTED;
        }
        throw std::invalid_argument("no tool data defined");
    }
    void setActive(Tool tool) {
        activeTool = tool;
    }
    bool isActive(Tool tool) {
        return activeTool == tool;
    }
    Tool getActiveTool() {
        return activeTool;
    }
    const ToolData& getActiveToolData() {
        return getData(activeTool);
    }
    NodeClickHandler::~NodeClickHandler() = default;
}
