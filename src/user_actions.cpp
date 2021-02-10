

#include "user_actions.h"

namespace user_actions {
    namespace {
        const std::vector<Action> ALL_ACTIONS = {
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
        };
    }

    void initialize() {
#ifdef NDEBUG
        for (int i = 0; i < ALL_ACTIONS.size(); ++i) {
            if (i != ALL_ACTIONS[i].id) {
                throw std::invalid_argument("user_actions::ALL_ACTION is unordered, please fix that (first wrong index is " + std::to_string(i) + ")");
            }
        }
#endif
    }

    void executeAction(int id) {
        ALL_ACTIONS[id].function();
    }

    const std::vector<Action> & findActionsByName(const std::string& name) {
        if (name.empty()){
            return ALL_ACTIONS;
        }
        //todo make better algorithm using leventshein distance or something similar
        static std::vector<Action> results;
        static std::string lastName;
        if (lastName != name) {
            results.clear();
            for (const auto &action : ALL_ACTIONS) {
                if (util::containsIgnoreCase(action.name, name)) {
                    results.push_back(action);
                }
            }
            lastName = name;
        }
        return results;
    }

    const std::vector<Action> &getAllActions() {
        return ALL_ACTIONS;
    }

    const Action &getAction(int id) {
        return ALL_ACTIONS[id];
    }
}