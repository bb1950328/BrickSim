#include "user_actions.h"
#include "controller.h"
#include "gui/gui.h"
#include "helpers/stringutil.h"
#include "helpers/util.h"
#include "lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include <magic_enum_utility.hpp>

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

        const std::array<ActionData, actionsCount> data = {{
                {
                        DO_NOTHING,
                        ICON_FA_BAN " Do nothing",
                        EnableCondition::ALWAYS,
                        []() {},
                },
                {
                        EXIT,
                        ICON_FA_DOOR_OPEN " Exit",
                        EnableCondition::ALWAYS,
                        []() { controller::setUserWantsToExit(true); },
                },
                {
                        OPEN_FILE,
                        ICON_FA_FOLDER_OPEN " Open",
                        EnableCondition::ALWAYS,
                        gui::showOpenFileDialog,
                },
                {
                        SAVE_FILE,
                        ICON_FA_FLOPPY_DISK " Save",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->save(); },
                },
                {
                        SAVE_FILE_AS,
                        ICON_FA_FLOPPY_DISK " Save as",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { gui::showSaveFileAsDialog(); },
                },
                {
                        SAVE_COPY_AS,
                        ICON_FA_COPY " Save copy as",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { gui::showSaveCopyAsDialog(); },
                },
                {
                        NEW_FILE,
                        ICON_FA_PLUS " New file",
                        EnableCondition::ALWAYS,
                        controller::createNewFile,
                },
                {
                        UNDO,
                        ICON_FA_ROTATE_LEFT " Undo",
                        EnableCondition::ALWAYS,
                        controller::undoLastAction,
                },
                {
                        REDO,
                        ICON_FA_ROTATE_RIGHT " Redo",
                        EnableCondition::ALWAYS,
                        controller::redoLastAction,
                },
                {
                        CUT,
                        ICON_FA_SCISSORS " Cut",
                        EnableCondition::HAS_SELECTED_NODES,
                        controller::cutSelectedObject,
                },
                {
                        COPY,
                        ICON_FA_COPY " Copy",
                        EnableCondition::HAS_SELECTED_NODES,
                        controller::copySelectedObject,
                },
                {
                        PASTE,
                        ICON_FA_PASTE " Paste",
                        EnableCondition::ALWAYS,
                        controller::pasteObject,
                },
                {
                        SELECT_ALL,
                        ICON_FA_SQUARE_CHECK " Select all",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->nodeSelectAll(); },
                },
                {
                        SELECT_NOTHING,
                        ICON_FA_SQUARE_MINUS " Select nothing",
                        EnableCondition::HAS_SELECTED_NODES,
                        []() { controller::getActiveEditor()->nodeSelectNone(); },
                },
                {
                        VIEW_3D_FRONT,
                        ICON_FA_DICE_ONE " Front view",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->setStandard3dView(1); },
                },
                {
                        VIEW_3D_TOP,
                        ICON_FA_DICE_TWO " Top view",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->setStandard3dView(2); },
                },
                {
                        VIEW_3D_RIGHT,
                        ICON_FA_DICE_THREE " Right view",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->setStandard3dView(3); },
                },
                {
                        VIEW_3D_REAR,
                        ICON_FA_DICE_FOUR " Rear view",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->setStandard3dView(4); },
                },
                {
                        VIEW_3D_BOTTOM,
                        ICON_FA_DICE_FIVE " Bottom view",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->setStandard3dView(5); },
                },
                {
                        VIEW_3D_LEFT,
                        ICON_FA_DICE_SIX " Left view",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->setStandard3dView(6); },
                },
                {
                        VIEW_3D_ROTATE_UP,
                        ICON_FA_ANGLE_UP " Rotate view upwards",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->rotateViewUp(); },
                },
                {
                        VIEW_3D_ROTATE_DOWN,
                        ICON_FA_ANGLE_DOWN " Rotate view downwards",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->rotateViewDown(); },
                },
                {
                        VIEW_3D_ROTATE_LEFT,
                        ICON_FA_ANGLE_LEFT " Rotate view left",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->rotateViewLeft(); },
                },
                {
                        VIEW_3D_ROTATE_RIGHT,
                        ICON_FA_ANGLE_RIGHT " Rotate view right",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->rotateViewRight(); },
                },
                {
                        VIEW_3D_PAN_UP,
                        ICON_FA_ANGLES_UP " Pan view upwards",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->panViewUp(); },
                },
                {
                        VIEW_3D_PAN_DOWN,
                        ICON_FA_ANGLES_DOWN " Pan view downwards",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->panViewDown(); },
                },
                {
                        VIEW_3D_PAN_LEFT,
                        ICON_FA_ANGLES_LEFT " Pan view left",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->panViewLeft(); },
                },
                {
                        VIEW_3D_PAN_RIGHT,
                        ICON_FA_ANGLES_RIGHT " Pan view right",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->panViewRight(); },
                },
                {
                        DELETE_SELECTED,
                        ICON_FA_TRASH_CAN " Delete selected element(s)",
                        EnableCondition::HAS_SELECTED_NODES,
                        []() { controller::getActiveEditor()->deleteSelectedElements(); },
                },
                {
                        HIDE_SELECTED,
                        ICON_FA_EYE_SLASH " Hide selected element(s)",
                        EnableCondition::HAS_SELECTED_NODES,
                        []() { controller::getActiveEditor()->hideSelectedElements(); },
                },
                {
                        UNHIDE_EVERYTHING,
                        ICON_FA_EYE " Unhide all elements",
                        EnableCondition::HAS_ACTIVE_EDITOR,
                        []() { controller::getActiveEditor()->unhideAllElements(); },
                },
                {
                        APPLY_DEFAULT_WINDOW_LAYOUT,
                        ICON_FA_TABLE_CELLS_LARGE " Apply default window layout",
                        EnableCondition::ALWAYS,
                        gui::applyDefaultWindowLayout,
                },
                {
                        TAKE_SCREENSHOT,
                        ICON_FA_CAMERA " Take screenshot",
                        EnableCondition::ALWAYS,
                        []() { gui::showScreenshotDialog(); },
                },
                {
                        EXECUTE_ACTION_BY_NAME,
                        ICON_FA_MAGNIFYING_GLASS " Find action",
                        EnableCondition::ALWAYS,
                        []() { gui::showExecuteActionByNameDialog(); },
                },
                {
                        TOGGLE_TRANSFORM_GIZMO_ROTATION,
                        "Toggle Transform Gizmo Rotation (World / Selected Element)",
                        EnableCondition::ALWAYS,
                        controller::toggleTransformGizmoRotationState,
                },
                {
                        SELECT_CONNECTED,
                        ICON_FA_PUZZLE_PIECE " Select connected",
                        EnableCondition::HAS_SELECTED_NODES,
                        []() { controller::getActiveEditor()->nodeSelectConnected(); },
                },
                {
                        INLINE_SELECTED_ELEMENTS,
                        ICON_FA_BARS " Inline Selected Elements",
                        EnableCondition::HAS_SELECTED_NODES,
                        []() { controller::getActiveEditor()->inlineSelectedElements(); },
                },
                {
                        START_TRANSLATING_SELECTED_NODES,
                        ICON_FA_UP_DOWN_LEFT_RIGHT " Start translating selected Elements",
                        EnableCondition::HAS_SELECTED_NODES,
                        []() { controller::getActiveEditor()->startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType::TRANSLATE); },
                },
                {
                        START_ROTATING_SELECTED_NODES,
                        ICON_FA_UP_DOWN_LEFT_RIGHT " Start rotating selected Elements",
                        EnableCondition::HAS_SELECTED_NODES,
                        []() { controller::getActiveEditor()->startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType::ROTATE); },
                },
                {
                        START_MOVING_SELECTED_NODES,
                        ICON_FA_UP_DOWN_LEFT_RIGHT " Start moving selected Elements",
                        EnableCondition::HAS_SELECTED_NODES,
                        []() { controller::getActiveEditor()->startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType::MOVE); },
                },
                {
                        TRANSFORMATION_LOCK_X,
                        ICON_FA_RULER_HORIZONTAL " Restrict transformation to X axis",
                        EnableCondition::TRANSFORM_IN_PROGRESS,
                        []() { controller::getActiveEditor()->getCurrentTransformAction()->toggleAxisLock(true, false, false); },
                },
                {
                        TRANSFORMATION_LOCK_Y,
                        ICON_FA_RULER_VERTICAL " Restrict transformation to Y axis",
                        EnableCondition::TRANSFORM_IN_PROGRESS,
                        []() { controller::getActiveEditor()->getCurrentTransformAction()->toggleAxisLock(false, true, false); },
                },
                {
                        TRANSFORMATION_LOCK_Z,
                        ICON_FA_RULER " Restrict transformation to Z axis",
                        EnableCondition::TRANSFORM_IN_PROGRESS,
                        []() { controller::getActiveEditor()->getCurrentTransformAction()->toggleAxisLock(false, false, true); },
                },
                {
                        TRANSFORMATION_LOCK_XY,
                        ICON_FA_RULER_COMBINED " Restrict transformation to X and Y axes",
                        EnableCondition::TRANSFORM_IN_PROGRESS,
                        []() { controller::getActiveEditor()->getCurrentTransformAction()->toggleAxisLock(true, true, false); },
                },
                {
                        TRANSFORMATION_LOCK_XZ,
                        ICON_FA_RULER_COMBINED " Restrict transformation to X and Z axes",
                        EnableCondition::TRANSFORM_IN_PROGRESS,
                        []() { controller::getActiveEditor()->getCurrentTransformAction()->toggleAxisLock(true, false, true); },
                },
                {
                        TRANSFORMATION_LOCK_YZ,
                        ICON_FA_RULER_COMBINED " Restrict transformation to Y and Z axes",
                        EnableCondition::TRANSFORM_IN_PROGRESS,
                        []() { controller::getActiveEditor()->getCurrentTransformAction()->toggleAxisLock(false, true, true); },
                },
                {
                        END_TRANSFORMATION,
                        ICON_FA_FORWARD_STEP " End transformation",
                        EnableCondition::TRANSFORM_IN_PROGRESS,
                        []() { controller::getActiveEditor()->endNodeTransformation(); },
                },
                {
                        CANCEL_TRANSFORMATION,
                        ICON_FA_CLOCK_ROTATE_LEFT " Cancel transformation",
                        EnableCondition::TRANSFORM_IN_PROGRESS,
                        []() { controller::getActiveEditor()->cancelNodeTransformation(); },
                },
        }};
    }

    void execute(Action action) {
        if (isEnabled(action)) {
            executeUnchecked(action);
        } else {
            throw std::invalid_argument("this action is not enabled right now");
        }
    }

    bool isEnabled(Action action) {
        switch (getData(action).enableCondition) {
            case EnableCondition::ALWAYS: return true;
            case EnableCondition::HAS_ACTIVE_EDITOR: return hasActiveEditor();
            case EnableCondition::TRANSFORM_IN_PROGRESS: return hasActiveEditor() && controller::getActiveEditor()->getCurrentTransformAction() != nullptr;
            case EnableCondition::HAS_SELECTED_NODES: return hasActiveEditor() && !controller::getActiveEditor()->getSelectedNodes().empty();
            default: return false;
        }
    }

    void executeUnchecked(Action action) {
        getData(action).function();
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
            for (const auto& d: data) {
                if (stringutil::containsIgnoreCase(d.name, name)) {
                    results.push_back(d.action);
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
        return stringutil::containsIgnoreCase(getData(action).name, filter);
    }

    const char* getName(const Action& action) {
        return getData(action).name;
    }
    void init() {
        ALL_ACTIONS.reserve(getCount());
        magic_enum::enum_for_each<Action>([](const auto& a) { ALL_ACTIONS.push_back(a); });
    }
    const ActionData& getData(const Action& action) {
        return data[action];
    }
    ActionData::ActionData(Action action,
                           const char* name,
                           EnableCondition enableCondition,
                           const std::function<void()>& function) :
        action(action),
        name(name),
        enableCondition(enableCondition),
        function(function) {}
}
