#include "user_actions.h"
#include "helpers/util.h"
#include "lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "controller.h"
#include "gui/gui.h"

namespace user_actions {
    namespace {
        std::vector<Action> ALL_ACTIONS;
    }

    void initialize() {
        ALL_ACTIONS.push_back(EXIT);
        ALL_ACTIONS.push_back(OPEN_FILE);
        ALL_ACTIONS.push_back(SAVE_FILE);
        ALL_ACTIONS.push_back(SAVE_FILE_AS);
        ALL_ACTIONS.push_back(SAVE_COPY_AS);
        ALL_ACTIONS.push_back(NEW_FILE);
        ALL_ACTIONS.push_back(UNDO);
        ALL_ACTIONS.push_back(REDO);
        ALL_ACTIONS.push_back(CUT);
        ALL_ACTIONS.push_back(COPY);
        ALL_ACTIONS.push_back(PASTE);
        ALL_ACTIONS.push_back(SELECT_ALL);
        ALL_ACTIONS.push_back(SELECT_NOTHING);
        ALL_ACTIONS.push_back(VIEW_3D_FRONT);
        ALL_ACTIONS.push_back(VIEW_3D_TOP);
        ALL_ACTIONS.push_back(VIEW_3D_RIGHT);
        ALL_ACTIONS.push_back(VIEW_3D_REAR);
        ALL_ACTIONS.push_back(VIEW_3D_BOTTOM);
        ALL_ACTIONS.push_back(VIEW_3D_LEFT);
        ALL_ACTIONS.push_back(VIEW_3D_ROTATE_UP);
        ALL_ACTIONS.push_back(VIEW_3D_ROTATE_DOWN);
        ALL_ACTIONS.push_back(VIEW_3D_ROTATE_LEFT);
        ALL_ACTIONS.push_back(VIEW_3D_ROTATE_RIGHT);
        ALL_ACTIONS.push_back(VIEW_3D_PAN_UP);
        ALL_ACTIONS.push_back(VIEW_3D_PAN_DOWN);
        ALL_ACTIONS.push_back(VIEW_3D_PAN_LEFT);
        ALL_ACTIONS.push_back(VIEW_3D_PAN_RIGHT);
        ALL_ACTIONS.push_back(DELETE_SELECTED);
        ALL_ACTIONS.push_back(HIDE_SELECTED);
        ALL_ACTIONS.push_back(UNHIDE_EVERYTHING);
        ALL_ACTIONS.push_back(APPLY_DEFAULT_WINDOW_LAYOUT);
        ALL_ACTIONS.push_back(TAKE_SCREENSHOT);
        ALL_ACTIONS.push_back(EXECUTE_ACTION_BY_NAME);
        ALL_ACTIONS.push_back(TOGGLE_TRANSFORM_GIZMO_ROTATION);
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

    extern const Action EXIT{0, "Exit", ICON_FA_SIGN_OUT_ALT " Exit", [](){controller::setUserWantsToExit(true);}};
    extern const Action OPEN_FILE{1, "Open", ICON_FA_FOLDER_OPEN " Open", gui::showOpenFileDialog};
    extern const Action SAVE_FILE{2, "Save", ICON_FA_SAVE" Save", controller::saveFile};
    extern const Action SAVE_FILE_AS{3, "Save as", ICON_FA_SAVE" Save as", gui::showSaveFileAsDialog};
    extern const Action SAVE_COPY_AS{4, "Save copy as", ICON_FA_COPY" Save copy as", gui::showSaveCopyAsDialog};
    extern const Action NEW_FILE{5, "New file", ICON_FA_PLUS" New file", controller::createNewFile};
    extern const Action UNDO{6, "Undo", ICON_FA_UNDO" Undo", controller::undoLastAction};
    extern const Action REDO{7, "Redo", ICON_FA_REDO" Redo", controller::redoLastAction};
    extern const Action CUT{8, "Cut", ICON_FA_CUT" Cut", controller::cutSelectedObject};
    extern const Action COPY{9, "Copy", ICON_FA_COPY" Copy", controller::copySelectedObject};
    extern const Action PASTE{10, "Paste", ICON_FA_PASTE" Paste", controller::pasteObject};
    extern const Action SELECT_ALL{11, "Select all", ICON_FA_CHECK_SQUARE" Select all", controller::nodeSelectAll};
    extern const Action SELECT_NOTHING{12, "Select nothing", ICON_FA_MINUS_SQUARE" Select nothing", controller::nodeSelectNone};
    extern const Action VIEW_3D_FRONT{13, "Front view", ICON_FA_DICE_ONE" Front view", [](){controller::setStandard3dView(1);}};
    extern const Action VIEW_3D_TOP{14, "Top view", ICON_FA_DICE_TWO" Top view", [](){controller::setStandard3dView(2);}};
    extern const Action VIEW_3D_RIGHT{15, "Right view", ICON_FA_DICE_THREE" Right view", [](){controller::setStandard3dView(3);}};
    extern const Action VIEW_3D_REAR{16, "Rear view", ICON_FA_DICE_FOUR" Rear view", [](){controller::setStandard3dView(4);}};
    extern const Action VIEW_3D_BOTTOM{17, "Bottom view", ICON_FA_DICE_FIVE" Bottom view", [](){controller::setStandard3dView(5);}};
    extern const Action VIEW_3D_LEFT{18, "Left view", ICON_FA_DICE_SIX" Left view", [](){controller::setStandard3dView(6);}};
    extern const Action VIEW_3D_ROTATE_UP{19, "Rotate view upwards", ICON_FA_ANGLE_UP" Rotate view upwards", controller::rotateViewUp};
    extern const Action VIEW_3D_ROTATE_DOWN{20, "Rotate view downwards", ICON_FA_ANGLE_DOWN" Rotate view downwards", controller::rotateViewDown};
    extern const Action VIEW_3D_ROTATE_LEFT{21, "Rotate view left", ICON_FA_ANGLE_LEFT" Rotate view left", controller::rotateViewLeft};
    extern const Action VIEW_3D_ROTATE_RIGHT{22, "Rotate view right", ICON_FA_ANGLE_RIGHT" Rotate view right", controller::rotateViewRight};
    extern const Action VIEW_3D_PAN_UP{23, "Pan view upwards", ICON_FA_ANGLE_DOUBLE_UP" Pan view upwards", controller::panViewUp};
    extern const Action VIEW_3D_PAN_DOWN{24, "Pan view downwards", ICON_FA_ANGLE_DOUBLE_DOWN" Pan view downwards", controller::panViewDown};
    extern const Action VIEW_3D_PAN_LEFT{25, "Pan view left", ICON_FA_ANGLE_DOUBLE_LEFT" Pan view left", controller::panViewLeft};
    extern const Action VIEW_3D_PAN_RIGHT{26, "Pan view right", ICON_FA_ANGLE_DOUBLE_RIGHT" Pan view right", controller::panViewRight};
    extern const Action DELETE_SELECTED{27, "Delete selected element(s)", ICON_FA_TRASH_ALT" Delete selected element(s)", controller::deleteSelectedElements};
    extern const Action HIDE_SELECTED{28, "Hide selected element(s)", ICON_FA_EYE_SLASH" Hide selected element(s)", controller::hideSelectedElements};
    extern const Action UNHIDE_EVERYTHING{29, "Unhide all elements", ICON_FA_EYE" Unhide all elements", controller::unhideAllElements};
    extern const Action APPLY_DEFAULT_WINDOW_LAYOUT{30, "Apply default window layout", ICON_FA_TH_LARGE" Apply default window layout", gui::applyDefaultWindowLayout};
    extern const Action TAKE_SCREENSHOT{31, "Take screenshot", ICON_FA_CAMERA" Take screenshot", gui::showScreenshotDialog};
    extern const Action EXECUTE_ACTION_BY_NAME{32, "Find action", ICON_FA_SEARCH" Find action", [](){gui::showExecuteActionByNameDialog();}};
    extern const Action TOGGLE_TRANSFORM_GIZMO_ROTATION{33, "Toggle Transform Gizmo Rotation (World / Selected Element)", "Toggle Transform Gizmo Rotation (World / Selected Element)", controller::toggleTransformGizmoRotationState};
}