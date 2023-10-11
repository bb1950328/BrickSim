#include "mouse_cursor_handler.h"
#include <spdlog/fmt/bundled/format.h>

namespace bricksim::gui {
    MouseCursor::MouseCursor(GLFWcursor* cursor) :
        cursor(cursor) {}
    MouseCursor::~MouseCursor() {
        glfwDestroyCursor(cursor);
    }
    void MouseCursor::activate(GLFWwindow* window) {
        glfwSetCursor(window, cursor);
    }
    StandardMouseCursor::StandardMouseCursor(StandardCursorType type) :
        MouseCursor(type == StandardCursorType::STANDARD ? nullptr : glfwCreateStandardCursor(static_cast<std::underlying_type_t<StandardCursorType>>(type))) {
    }
    IconMouseCursor::IconMouseCursor(icons::IconType type) :
        MouseCursor(createCursor(type)) {
    }
    GLFWcursor* IconMouseCursor::createCursor(icons::IconType type) {
        auto imageData = icons::getRawImage(type, icons::Icon16);
        GLFWimage image = {
                .width = imageData.width,
                .height = imageData.height,
                .pixels = imageData.data.data(),
        };
        const auto cursor = glfwCreateCursor(&image, 0, 0);
        if (cursor == nullptr) {
            throw std::invalid_argument(fmt::format("cannot create icon cursor for {}", magic_enum::enum_name(type)));
        }
        return cursor;
    }

    StandardMouseCursor& MouseCursorHandler::getStandardCursor() {
        return getStandardCursor(StandardCursorType::STANDARD);
    }

    StandardMouseCursor& MouseCursorHandler::getStandardCursor(StandardCursorType type) {
        const auto it = standardCursors.find(type);
        if (it == standardCursors.end()) {
            return standardCursors.emplace(type, StandardMouseCursor(type)).first->second;
        }
        return it->second;
    }

    IconMouseCursor& MouseCursorHandler::getIconCursor(icons::IconType type) {
        const auto it = iconCursors.find(type);
        if (it == iconCursors.end()) {
            return iconCursors.emplace(type, IconMouseCursor(type)).first->second;
        }
        return it->second;
    }
}
