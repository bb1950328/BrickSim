#include "mouse_cursor_handler.h"
#include "../controller.h"
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/spdlog.h>

namespace bricksim::gui {
    struct GLFWcursorDestroyer {
        void operator()(GLFWcursor* cursor) {
            controller::executeOpenGL([cursor]() {
                glfwDestroyCursor(cursor);
                spdlog::trace("destroyed cursor {}", fmt::ptr(cursor));
            });
        }
    };

    MouseCursor::MouseCursor(GLFWcursor* cursor) :
        cursor(std::shared_ptr<GLFWcursor>(cursor, GLFWcursorDestroyer{})) {}

    void MouseCursor::activate(GLFWwindow* window) {
        glfwSetCursor(window, cursor.get());
    }

    MouseCursor createStandard(StandardCursorType type) {
        auto* cursor = type == StandardCursorType::STANDARD
                               ? nullptr
                               : glfwCreateStandardCursor(static_cast<std::underlying_type_t<StandardCursorType>>(type));
        spdlog::trace("created standard cursor {} StandardCursorType::{}", fmt::ptr(cursor), magic_enum::enum_name(type));
        return MouseCursor(cursor);
    }

    MouseCursor createIcon(icons::IconType type) {
        constexpr auto size = icons::Icon48;
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
        spdlog::trace("created icon cursor {} IconType::{}", fmt::ptr(cursor), magic_enum::enum_name(type));
        return MouseCursor(cursor);
    }

    MouseCursor& MouseCursorHandler::getStandardCursor(StandardCursorType type) {
        const auto it = standardCursors.find(type);
        if (it == standardCursors.end()) {
            return standardCursors.emplace(type, createStandard(type)).first->second;
        }
        return it->second;
    }

    MouseCursor& MouseCursorHandler::getIconCursor(icons::IconType type) {
        const auto it = iconCursors.find(type);
        if (it == iconCursors.end()) {
            return iconCursors.emplace(type, createIcon(type)).first->second;
        }
        return it->second;
    }

    MouseCursor& MouseCursorHandler::getStandardCursor() {
        return getStandardCursor(StandardCursorType::STANDARD);
    }
}
