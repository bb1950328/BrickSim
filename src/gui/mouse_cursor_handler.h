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

    class MouseCursor : public util::NoCopyNoMove {
    private:
        GLFWcursor* cursor;

    public:
        explicit MouseCursor(GLFWcursor* cursor);
        virtual ~MouseCursor();
        void activate(GLFWwindow* window);
    };

    class StandardMouseCursor : public MouseCursor {
    public:
        explicit StandardMouseCursor(StandardCursorType type);
    };

    class IconMouseCursor : public MouseCursor {
    public:
        explicit IconMouseCursor(icons::IconType type);

    protected:
        static GLFWcursor* createCursor(icons::IconType type);
    };

    class MouseCursorHandler : public util::NoCopyNoMove {
    private:
        uomap_t<StandardCursorType, StandardMouseCursor> standardCursors;
        uomap_t<icons::IconType, IconMouseCursor> iconCursors;

    public:
        StandardMouseCursor& getStandardCursor();
        StandardMouseCursor& getStandardCursor(StandardCursorType type);
        IconMouseCursor& getIconCursor(icons::IconType type);
    };
}
