#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#pragma once

#include <functional>
#include <magic_enum.hpp>
#include <string>

namespace bricksim::user_actions {
    enum Action : uint32_t {
        DO_NOTHING,
        EXIT,
        OPEN_FILE,
        SAVE_FILE,
        SAVE_FILE_AS,
        SAVE_COPY_AS,
        NEW_FILE,
        UNDO,
        REDO,
        CUT,
        COPY,
        PASTE,
        SELECT_ALL,
        SELECT_NOTHING,
        VIEW_3D_FRONT,
        VIEW_3D_TOP,
        VIEW_3D_RIGHT,
        VIEW_3D_REAR,
        VIEW_3D_BOTTOM,
        VIEW_3D_LEFT,
        VIEW_3D_ROTATE_UP,
        VIEW_3D_ROTATE_DOWN,
        VIEW_3D_ROTATE_LEFT,
        VIEW_3D_ROTATE_RIGHT,
        VIEW_3D_PAN_UP,
        VIEW_3D_PAN_DOWN,
        VIEW_3D_PAN_LEFT,
        VIEW_3D_PAN_RIGHT,
        DELETE_SELECTED,
        HIDE_SELECTED,
        UNHIDE_EVERYTHING,
        APPLY_DEFAULT_WINDOW_LAYOUT,
        TAKE_SCREENSHOT,
        EXECUTE_ACTION_BY_NAME,
        TOGGLE_TRANSFORM_GIZMO_ROTATION,
    };

    constexpr size_t getCount();
    void execute(Action action);
    void executeUnchecked(Action action);
    bool isEnabled(Action action);
    bool isInFilter(Action action, const std::string& filter);
    const std::vector<Action>& findActionsByName(const std::string& name);
    const char* getName(const Action& action);
}

#pragma clang diagnostic pop
