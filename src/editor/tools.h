#pragma once

#include "../gui/icons.h"
#include "editor.h"
namespace bricksim::tools {
    enum class Tool {
        SELECT,
        SELECT_CONNECTED,
        SELECT_STRONGLY_CONNECTED,
    };

    struct NodeClickHandler {
        virtual void operator()(const std::shared_ptr<Editor>& editor, const std::shared_ptr<etree::Node>& node, bool ctrl, bool shift) = 0;
        virtual ~NodeClickHandler();
    };

    class ToolData : util::NoCopyNoMove {
    public:
        const Tool tool;
        const std::string_view name;
        const gui::icons::IconType icon;
        const std::string nameWithIcon;
        const std::unique_ptr<NodeClickHandler> handleNodeClicked;

        ToolData(Tool tool,
                 const std::string_view& name,
                 const gui::icons::IconType icon,
                 std::unique_ptr<NodeClickHandler> handleNodeClicked);
    };

    const ToolData& getData(Tool tool);
    void setActive(Tool tool);
    bool isActive(Tool tool);
    Tool getActiveTool();
    const ToolData& getActiveToolData();

}
