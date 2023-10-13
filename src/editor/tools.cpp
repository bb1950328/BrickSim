#include "tools.h"
#include <spdlog/fmt/bundled/format.h>

namespace bricksim::tools {
    namespace {
        using namespace std::string_view_literals;
        const ToolData SELECT = ToolData(Tool::SELECT, "Select"sv, gui::icons::IconType::Select);
        const ToolData SELECT_CONNECTED = ToolData(Tool::SELECT_CONNECTED, "Select Connected"sv, gui::icons::IconType::SelectConnected);
        const ToolData SELECT_STRONGLY_CONNECTED = ToolData(Tool::SELECT_STRONGLY_CONNECTED, "Select Strongly Connected"sv, gui::icons::IconType::SelectStronglyConnected);

        Tool activeTool = Tool::SELECT;
    }

    ToolData::ToolData(Tool tool, const std::string_view& name, const gui::icons::IconType icon) :
        tool(tool), name(name), icon(icon), nameWithIcon(fmt::format("{}{}", gui::icons::getGlyph(icon, gui::icons::Icon36), name)) {
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
}
