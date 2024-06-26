#pragma once

#include "editor/editor.h"
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
        SELECT_CONNECTED,
        INLINE_SELECTED_ELEMENTS,
        START_TRANSLATING_SELECTED_NODES,
        START_ROTATING_SELECTED_NODES,
        START_MOVING_SELECTED_NODES,
        TRANSFORMATION_LOCK_X,
        TRANSFORMATION_LOCK_Y,
        TRANSFORMATION_LOCK_Z,
        TRANSFORMATION_LOCK_XY,
        TRANSFORMATION_LOCK_XZ,
        TRANSFORMATION_LOCK_YZ,
        END_TRANSFORMATION,
        CANCEL_TRANSFORMATION,
    };

    enum class EnableCondition {
        ALWAYS,
        HAS_ACTIVE_EDITOR,
        TRANSFORM_IN_PROGRESS,
        HAS_SELECTED_NODES,
    };

    constexpr size_t getCount();
    void execute(Action action, std::shared_ptr<Editor> editorContext);
    void executeUnchecked(Action action, std::shared_ptr<Editor> editorContext);
    bool isEnabled(Action action);
    bool isInFilter(Action action, const std::string& filter);
    const std::vector<Action>& findActionsByName(const std::string& name);
    std::string_view getName(const Action& action);
    void init();
}
