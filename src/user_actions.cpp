#include "user_actions.h"
#include "controller.h"
#include "gui/gui.h"
#include "helpers/stringutil.h"
#include "helpers/util.h"
#include "lib/IconFontCppHeaders/IconsFontAwesome6.h"

namespace bricksim::user_actions {
    namespace {
        std::vector<Action> ALL_ACTIONS;

        constexpr bool alwaysTrue() {
            return true;
        }

        bool hasActiveEditor() {
            return controller::getActiveEditor() != nullptr;
        }

        constexpr auto actionsCount = magic_enum::enum_count<Action>();

        constexpr std::array<const char* const, actionsCount> names{
                ICON_FA_BAN " Do nothing",
                ICON_FA_DOOR_OPEN " Exit",
                ICON_FA_FOLDER_OPEN " Open",
                ICON_FA_FLOPPY_DISK " Save",
                ICON_FA_FLOPPY_DISK " Save as",
                ICON_FA_COPY " Save copy as",
                ICON_FA_PLUS " New file",
                ICON_FA_ROTATE_LEFT " Undo",
                ICON_FA_ROTATE_RIGHT " Redo",
                ICON_FA_SCISSORS " Cut",
                ICON_FA_COPY " Copy",
                ICON_FA_PASTE " Paste",
                ICON_FA_SQUARE_CHECK " Select all",
                ICON_FA_SQUARE_MINUS " Select nothing",
                ICON_FA_DICE_ONE " Front view",
                ICON_FA_DICE_TWO " Top view",
                ICON_FA_DICE_THREE " Right view",
                ICON_FA_DICE_FOUR " Rear view",
                ICON_FA_DICE_FIVE " Bottom view",
                ICON_FA_DICE_SIX " Left view",
                ICON_FA_ANGLE_UP " Rotate view upwards",
                ICON_FA_ANGLE_DOWN " Rotate view downwards",
                ICON_FA_ANGLE_LEFT " Rotate view left",
                ICON_FA_ANGLE_RIGHT " Rotate view right",
                ICON_FA_ANGLES_UP " Pan view upwards",
                ICON_FA_ANGLES_DOWN " Pan view downwards",
                ICON_FA_ANGLES_LEFT " Pan view left",
                ICON_FA_ANGLES_RIGHT " Pan view right",
                ICON_FA_TRASH_CAN " Delete selected element(s)",
                ICON_FA_EYE_SLASH " Hide selected element(s)",
                ICON_FA_EYE " Unhide all elements",
                ICON_FA_TABLE_CELLS_LARGE " Apply default window layout",
                ICON_FA_CAMERA " Take screenshot",
                ICON_FA_MAGNIFYING_GLASS " Find action",
                "Toggle Transform Gizmo Rotation (World / Selected Element)",
                ICON_FA_PUZZLE_PIECE " Select connected",
                ICON_FA_BARS " Inline Selected Elements",
        };

        const std::array<std::function<bool()>, actionsCount> actionEnabledFuncs{
                alwaysTrue,     //DO_NOTHING
                alwaysTrue,     //EXIT
                alwaysTrue,     //OPEN_FILE
                hasActiveEditor,//SAVE_FILE
                hasActiveEditor,//SAVE_FILE_AS
                hasActiveEditor,//SAVE_COPY_AS
                alwaysTrue,     //NEW_FILE
                alwaysTrue,     //UNDO
                alwaysTrue,     //REDO
                alwaysTrue,     //CUT
                alwaysTrue,     //COPY
                alwaysTrue,     //PASTE
                hasActiveEditor,//SELECT_ALL
                hasActiveEditor,//SELECT_NOTHING
                hasActiveEditor,//VIEW_3D_FRONT
                hasActiveEditor,//VIEW_3D_TOP
                hasActiveEditor,//VIEW_3D_RIGHT
                hasActiveEditor,//VIEW_3D_REAR
                hasActiveEditor,//VIEW_3D_BOTTOM
                hasActiveEditor,//VIEW_3D_LEFT
                hasActiveEditor,//VIEW_3D_ROTATE_UP
                hasActiveEditor,//VIEW_3D_ROTATE_DOWN
                hasActiveEditor,//VIEW_3D_ROTATE_LEFT
                hasActiveEditor,//VIEW_3D_ROTATE_RIGHT
                hasActiveEditor,//VIEW_3D_PAN_UP
                hasActiveEditor,//VIEW_3D_PAN_DOWN
                hasActiveEditor,//VIEW_3D_PAN_LEFT
                hasActiveEditor,//VIEW_3D_PAN_RIGHT
                hasActiveEditor,//DELETE_SELECTED
                hasActiveEditor,//HIDE_SELECTED
                hasActiveEditor,//UNHIDE_EVERYTHING
                alwaysTrue,     //APPLY_DEFAULT_WINDOW_LAYOUT
                alwaysTrue,     //TAKE_SCREENSHOT
                alwaysTrue,     //EXECUTE_ACTION_BY_NAME
                alwaysTrue,     //TOGGLE_TRANSFORM_GIZMO_ROTATION
                hasActiveEditor,//SELECT_CONNECTED
                hasActiveEditor,//INLINE_SELECTED_ELEMENTS
        };

        const std::array<std::function<void()>, actionsCount> functions{
                []() {},                                                          //DO_NOTHING
                []() { controller::setUserWantsToExit(true); },                   //EXIT
                gui::showOpenFileDialog,                                          //OPEN_FILE
                []() { controller::getActiveEditor()->save(); },                  //SAVE_FILE
                []() { gui::showSaveFileAsDialog(); },                            //SAVE_FILE_AS
                []() { gui::showSaveCopyAsDialog(); },                            //SAVE_COPY_AS
                controller::createNewFile,                                        //NEW_FILE
                controller::undoLastAction,                                       //UNDO
                controller::redoLastAction,                                       //REDO
                controller::cutSelectedObject,                                    //CUT
                controller::copySelectedObject,                                   //COPY
                controller::pasteObject,                                          //PASTE
                []() { controller::getActiveEditor()->nodeSelectAll(); },         //SELECT_ALL
                []() { controller::getActiveEditor()->nodeSelectNone(); },        //SELECT_NOTHING
                []() { controller::getActiveEditor()->setStandard3dView(1); },    //VIEW_3D_FRONT
                []() { controller::getActiveEditor()->setStandard3dView(2); },    //VIEW_3D_TOP
                []() { controller::getActiveEditor()->setStandard3dView(3); },    //VIEW_3D_RIGHT
                []() { controller::getActiveEditor()->setStandard3dView(4); },    //VIEW_3D_REAR
                []() { controller::getActiveEditor()->setStandard3dView(5); },    //VIEW_3D_BOTTOM
                []() { controller::getActiveEditor()->setStandard3dView(6); },    //VIEW_3D_LEFT
                []() { controller::getActiveEditor()->rotateViewUp(); },          //VIEW_3D_ROTATE_UP
                []() { controller::getActiveEditor()->rotateViewDown(); },        //VIEW_3D_ROTATE_DOWN
                []() { controller::getActiveEditor()->rotateViewLeft(); },        //VIEW_3D_ROTATE_LEFT
                []() { controller::getActiveEditor()->rotateViewRight(); },       //VIEW_3D_ROTATE_RIGHT
                []() { controller::getActiveEditor()->panViewUp(); },             //VIEW_3D_PAN_UP
                []() { controller::getActiveEditor()->panViewDown(); },           //VIEW_3D_PAN_DOWN
                []() { controller::getActiveEditor()->panViewLeft(); },           //VIEW_3D_PAN_LEFT
                []() { controller::getActiveEditor()->panViewRight(); },          //VIEW_3D_PAN_RIGHT
                []() { controller::getActiveEditor()->deleteSelectedElements(); },//DELETE_SELECTED
                []() { controller::getActiveEditor()->hideSelectedElements(); },  //HIDE_SELECTED
                []() { controller::getActiveEditor()->unhideAllElements(); },     //UNHIDE_EVERYTHING
                gui::applyDefaultWindowLayout,                                    //APPLY_DEFAULT_WINDOW_LAYOUT
                []() { gui::showScreenshotDialog(); },                            //TAKE_SCREENSHOT
                []() { gui::showExecuteActionByNameDialog(); },                   //EXECUTE_ACTION_BY_NAME
                controller::toggleTransformGizmoRotationState,                    //TOGGLE_TRANSFORM_GIZMO_ROTATION
                []() { controller::getActiveEditor()->nodeSelectConnected(); },   //SELECT_CONNECTED
                []() { controller::getActiveEditor()->inlineSelectedElements(); },   //INLINE_SELECTED_ELEMENTS
        };
    }

    void execute(Action action) {
        if (isEnabled(action)) {
            executeUnchecked(action);
        } else {
            throw std::invalid_argument("this action is not enabled right now");
        }
    }

    bool isEnabled(Action action) {
        return actionEnabledFuncs[action]();
    }

    void executeUnchecked(Action action) {
        functions[action]();
    }

    const std::vector<Action>& findActionsByName(const std::string& name) {
        if (name.empty()) {
            return ALL_ACTIONS;
        }
        //todo make better algorithm using leventshein distance or something similar
        static std::vector<Action> results;
        static std::string lastName;
        if (lastName != name) {
            results.clear();
            for (size_t i = 0; i < actionsCount; ++i) {
                if (stringutil::containsIgnoreCase(names[i], name)) {
                    results.push_back(static_cast<Action>(i));
                }
            }
            lastName = name;
        }
        return results;
    }
    constexpr size_t getCount() {
        return actionsCount;
    }

    bool isInFilter(Action action, const std::string& filter) {
        //todo make this typo-robust
        return stringutil::containsIgnoreCase(names[action], filter);
    }

    const char* getName(const Action& action) {
        return names[action];
    }
    void init() {
        ALL_ACTIONS.reserve(getCount());
        magic_enum::enum_for_each<Action>([](const auto& a) { ALL_ACTIONS.push_back(a); });
    }
}
