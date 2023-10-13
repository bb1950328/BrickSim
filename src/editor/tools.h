#pragma once

#include "../gui/icons.h"
namespace bricksim::tools {
    enum class Tool {
        SELECT,
        SELECT_CONNECTED,
        SELECT_STRONGLY_CONNECTED,
    };

    class ToolData : util::NoCopyNoMove {
    public:
        const Tool tool;
        const std::string_view name;
        const gui::icons::IconType icon;
        const std::string nameWithIcon;

        ToolData(Tool tool, const std::string_view& name, const gui::icons::IconType icon);
    };

    const ToolData& getData(Tool tool);
    void setActive(Tool tool);
    bool isActive(Tool tool);
    Tool getActiveTool();
}
