#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#ifndef BRICKSIM_USER_ACTIONS_H
#define BRICKSIM_USER_ACTIONS_H

#include <functional>
#include <string>

namespace user_actions {
    struct Action {
        const int id;
        const char* name;
        const char* nameWithIcon;
        std::function<void()> function;
    };
    // add new user actions here and in user_actions.cpp
    // IMPORTANT: only append at the end, because otherwise the id's get messed up
    extern const Action EXIT;
    extern const Action OPEN_FILE;
    extern const Action SAVE_FILE;
    extern const Action SAVE_FILE_AS;
    extern const Action SAVE_COPY_AS;
    extern const Action NEW_FILE;
    extern const Action UNDO;
    extern const Action REDO;
    extern const Action CUT;
    extern const Action COPY;
    extern const Action PASTE;
    extern const Action SELECT_ALL;
    extern const Action SELECT_NOTHING;
    extern const Action VIEW_3D_FRONT;
    extern const Action VIEW_3D_TOP;
    extern const Action VIEW_3D_RIGHT;
    extern const Action VIEW_3D_REAR;
    extern const Action VIEW_3D_BOTTOM;
    extern const Action VIEW_3D_LEFT;
    extern const Action VIEW_3D_ROTATE_UP;
    extern const Action VIEW_3D_ROTATE_DOWN;
    extern const Action VIEW_3D_ROTATE_LEFT;
    extern const Action VIEW_3D_ROTATE_RIGHT;
    extern const Action VIEW_3D_PAN_UP;
    extern const Action VIEW_3D_PAN_DOWN;
    extern const Action VIEW_3D_PAN_LEFT;
    extern const Action VIEW_3D_PAN_RIGHT;
    extern const Action DELETE_SELECTED;
    extern const Action HIDE_SELECTED;
    extern const Action UNHIDE_EVERYTHING;
    extern const Action APPLY_DEFAULT_WINDOW_LAYOUT;
    extern const Action TAKE_SCREENSHOT;
    extern const Action EXECUTE_ACTION_BY_NAME;
    extern const Action TOGGLE_TRANSFORM_GIZMO_ROTATION;

    void initialize();
    void executeAction(int id);
    const std::vector<Action> & findActionsByName(const std::string& name);
    const std::vector<Action>& getAllActions();
    const Action& getAction(int id);
}
#endif //BRICKSIM_USER_ACTIONS_H

#pragma clang diagnostic pop