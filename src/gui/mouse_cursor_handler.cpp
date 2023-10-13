#include "mouse_cursor_handler.h"
#include "../controller.h"
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/spdlog.h>

namespace bricksim::gui {
    MouseCursor::MouseCursor(GLFWcursor* cursor) :
        cursor(cursor) {}
    MouseCursor::~MouseCursor() {
        controller::executeOpenGL([this]() {
            glfwDestroyCursor(cursor);
            spdlog::trace("destroyed cursor {}", fmt::ptr(cursor));
        });
    }
    void MouseCursor::activate(GLFWwindow* window) {
        glfwSetCursor(window, cursor);
    }
    MouseCursor::MouseCursor(MouseCursor&& other) :
        cursor(other.cursor) {
    }
    StandardMouseCursor::StandardMouseCursor(StandardCursorType type) :
        MouseCursor(type == StandardCursorType::STANDARD ? nullptr : glfwCreateStandardCursor(static_cast<std::underlying_type_t<StandardCursorType>>(type))) {
        spdlog::trace("created cursor {} StandardCursorType::{}", fmt::ptr(cursor), magic_enum::enum_name(type));
    }
    IconMouseCursor::IconMouseCursor(icons::IconType type) :
        MouseCursor(createCursor(type)) {
        spdlog::trace("created cursor {} IconType::{}", fmt::ptr(cursor), magic_enum::enum_name(type));
    }

    GLFWcursor* IconMouseCursor::createCursor(icons::IconType type) {
        auto imageData = icons::getRawImage(type, size);
        const auto hotPoint = icons::getHotPoint(type, size);
        GLFWimage image = {
                .width = imageData.width,
                .height = imageData.height,
                .pixels = imageData.data.data(),
        };
        GLFWcursor* cursor;
        controller::executeOpenGL([&image, &cursor, &hotPoint]() {
            cursor = glfwCreateCursor(&image, hotPoint.x, hotPoint.y);
        });
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
            return standardCursors.emplace(type, type).first->second;
        }
        return it->second;
    }

    IconMouseCursor& MouseCursorHandler::getIconCursor(icons::IconType type) {
        const auto it = iconCursors.find(type);
        if (it == iconCursors.end()) {
            return iconCursors.emplace(type, type).first->second;
        }
        return it->second;
    }
}
