#pragma once
#include "../types.h"
#include "GLFW/glfw3.h"
#include "icons.h"
#include <vector>

namespace bricksim::gui {
    enum class StandardCursorType : int {
        STANDARD = 0,
        ARROW = GLFW_ARROW_CURSOR,
        IBEAM = GLFW_IBEAM_CURSOR,
        CROSSHAIR = GLFW_CROSSHAIR_CURSOR,
        POINTING_HAND = GLFW_POINTING_HAND_CURSOR,
        RESIZE_EW = GLFW_RESIZE_EW_CURSOR,
        RESIZE_NS = GLFW_RESIZE_NS_CURSOR,
        RESIZE_NWSE = GLFW_RESIZE_NWSE_CURSOR,
        RESIZE_NESW = GLFW_RESIZE_NESW_CURSOR,
        RESIZE_ALL = GLFW_RESIZE_ALL_CURSOR,
        NOT_ALLOWED = GLFW_NOT_ALLOWED_CURSOR,
    };

    class MouseCursor {
    protected:
        std::shared_ptr<GLFWcursor> cursor;

    public:
        explicit MouseCursor(GLFWcursor* cursor);
        void activate(GLFWwindow* window);
    };

    class MouseCursorHandler : public util::NoCopy {
    private:
        uomap_t<StandardCursorType, MouseCursor> standardCursors;
        uomap_t<icons::IconType, MouseCursor> iconCursors;

    public:
        MouseCursor& getStandardCursor();
        MouseCursor& getStandardCursor(StandardCursorType type);
        MouseCursor& getIconCursor(icons::IconType type);
    };
}
