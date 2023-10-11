#pragma once

#include "../gui/icons.h"
namespace bricksim::tools {
    class ToolData {
    public:
        const std::string_view name;
        const gui::icons::IconType icon;
        const std::string nameWithIcon;

        ToolData(const std::string_view& name, const gui::icons::IconType icon);
    };

    enum class Tool {
        SELECT,
        SELECT_CONNECTED,
        SELECT_STRONGLY_CONNECTED,
    };

    const ToolData& getData(Tool tool);
    void setActive(Tool tool);
    bool isActive(Tool tool);
    Tool getActiveTool();
}
