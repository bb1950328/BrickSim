#include "user_actions.h"
#include "controller.h"
#include "gui/dialogs.h"
#include "gui/gui.h"
#include "helpers/stringutil.h"
#include "helpers/util.h"
#include "lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "spdlog/spdlog.h"
#include <magic_enum_utility.hpp>

namespace bricksim::user_actions {
    namespace {
        using namespace std::literals;

        std::vector<Action> ALL_ACTIONS;

        constexpr bool alwaysTrue() {
            return true;
        }

        bool hasActiveEditor() {
            return controller::getActiveEditor() != nullptr;
        }

        #if NDEBUG
        using ActionName = std::string_view;
        constexpr ActionName actionName(Action ignored, std::string_view name) {
            return name;
        }
        #else
        struct ActionName {
            Action action;
            std::string_view name;
        };

        constexpr ActionName actionName(Action action, std::string_view name) {
            return {action, name};
        }
        #endif

        constexpr auto actionsCount = magic_enum::enum_count<Action>();

        constexpr std::array<ActionName, actionsCount> names = {{
                actionName(DO_NOTHING, ICON_FA_BAN " Do nothing"sv),
                actionName(EXIT, ICON_FA_DOOR_OPEN " Exit"sv),
                actionName(OPEN_FILE, ICON_FA_FOLDER_OPEN " Open"sv),
                actionName(SAVE_FILE, ICON_FA_FLOPPY_DISK " Save"sv),
                actionName(SAVE_FILE_AS, ICON_FA_FLOPPY_DISK " Save as"sv),
                actionName(SAVE_COPY_AS, ICON_FA_COPY " Save copy as"sv),
                actionName(NEW_FILE, ICON_FA_PLUS " New file"sv),
                actionName(UNDO, ICON_FA_ROTATE_LEFT " Undo"sv),
                actionName(REDO, ICON_FA_ROTATE_RIGHT " Redo"sv),
                actionName(CUT, ICON_FA_SCISSORS " Cut"sv),
                actionName(COPY, ICON_FA_COPY " Copy"sv),
                actionName(PASTE, ICON_FA_PASTE " Paste"sv),
                actionName(SELECT_ALL, ICON_FA_SQUARE_CHECK " Select all"sv),
                actionName(SELECT_NOTHING, ICON_FA_SQUARE_MINUS " Select nothing"sv),
                actionName(VIEW_3D_FRONT, ICON_FA_DICE_ONE " Front view"sv),
                actionName(VIEW_3D_TOP, ICON_FA_DICE_TWO " Top view"sv),
                actionName(VIEW_3D_RIGHT, ICON_FA_DICE_THREE " Right view"sv),
                actionName(VIEW_3D_REAR, ICON_FA_DICE_FOUR " Rear view"sv),
                actionName(VIEW_3D_BOTTOM, ICON_FA_DICE_FIVE " Bottom view"sv),
                actionName(VIEW_3D_LEFT, ICON_FA_DICE_SIX " Left view"sv),
                actionName(VIEW_3D_ROTATE_UP, ICON_FA_ANGLE_UP " Rotate view upwards"sv),
                actionName(VIEW_3D_ROTATE_DOWN, ICON_FA_ANGLE_DOWN " Rotate view downwards"sv),
                actionName(VIEW_3D_ROTATE_LEFT, ICON_FA_ANGLE_LEFT " Rotate view left"sv),
                actionName(VIEW_3D_ROTATE_RIGHT, ICON_FA_ANGLE_RIGHT " Rotate view right"sv),
                actionName(VIEW_3D_PAN_UP, ICON_FA_ANGLES_UP " Pan view upwards"sv),
                actionName(VIEW_3D_PAN_DOWN, ICON_FA_ANGLES_DOWN " Pan view downwards"sv),
                actionName(VIEW_3D_PAN_LEFT, ICON_FA_ANGLES_LEFT " Pan view left"sv),
                actionName(VIEW_3D_PAN_RIGHT, ICON_FA_ANGLES_RIGHT " Pan view right"sv),
                actionName(DELETE_SELECTED, ICON_FA_TRASH_CAN " Delete selected element(s)"sv),
                actionName(HIDE_SELECTED, ICON_FA_EYE_SLASH " Hide selected element(s)"sv),
                actionName(UNHIDE_EVERYTHING, ICON_FA_EYE " Unhide all elements"sv),
                actionName(APPLY_DEFAULT_WINDOW_LAYOUT, ICON_FA_TABLE_CELLS_LARGE " Apply default window layout"sv),
                actionName(TAKE_SCREENSHOT, ICON_FA_CAMERA " Take screenshot"sv),
                actionName(EXECUTE_ACTION_BY_NAME, ICON_FA_MAGNIFYING_GLASS " Find action"sv),
                actionName(TOGGLE_TRANSFORM_GIZMO_ROTATION, "Toggle Transform Gizmo Rotation (World / Selected Element)"sv),
                actionName(SELECT_CONNECTED, ICON_FA_SHARE_NODES " Select connected"sv),
                actionName(INLINE_SELECTED_ELEMENTS, ICON_FA_BARS " Inline Selected Elements"sv),
                actionName(START_TRANSLATING_SELECTED_NODES, ICON_FA_UP_DOWN_LEFT_RIGHT " Start translating selected Elements"sv),
                actionName(START_ROTATING_SELECTED_NODES, ICON_FA_UP_DOWN_LEFT_RIGHT " Start rotating selected Elements"sv),
                actionName(START_MOVING_SELECTED_NODES, ICON_FA_UP_DOWN_LEFT_RIGHT " Start moving selected Elements"sv),
                actionName(TRANSFORMATION_LOCK_X, ICON_FA_RULER_HORIZONTAL " Restrict transformation to X axis"sv),
                actionName(TRANSFORMATION_LOCK_Y, ICON_FA_RULER_VERTICAL " Restrict transformation to Y axis"sv),
                actionName(TRANSFORMATION_LOCK_Z, ICON_FA_RULER " Restrict transformation to Z axis"sv),
                actionName(TRANSFORMATION_LOCK_XY, ICON_FA_RULER_COMBINED " Restrict transformation to X and Y axes"sv),
                actionName(TRANSFORMATION_LOCK_XZ, ICON_FA_RULER_COMBINED " Restrict transformation to X and Z axes"sv),
                actionName(TRANSFORMATION_LOCK_YZ, ICON_FA_RULER_COMBINED " Restrict transformation to Y and Z axes"sv),
                actionName(END_TRANSFORMATION, ICON_FA_FORWARD_STEP " End transformation"sv),
                actionName(CANCEL_TRANSFORMATION, ICON_FA_CLOCK_ROTATE_LEFT " Cancel transformation"sv),
        }};
    }

    void execute(Action action, std::shared_ptr<Editor> editorContext) {
        if (isEnabled(action)) {
            executeUnchecked(action, editorContext);
        } else {
            throw std::invalid_argument("this action is not enabled right now");
        }
    }

    bool hasSelectedNodes() {
        return hasActiveEditor() && !controller::getActiveEditor()->getSelectedNodes().empty();
    }

    bool hasTransformInProgress() {
        return hasActiveEditor() && controller::getActiveEditor()->getCurrentTransformAction() != nullptr;
    }

    bool isEnabled(Action action) {
        switch (action) {
            case SAVE_FILE:
            case SAVE_FILE_AS:
            case SAVE_COPY_AS:
            case SELECT_ALL:
            case VIEW_3D_FRONT:
            case VIEW_3D_TOP:
            case VIEW_3D_RIGHT:
            case VIEW_3D_REAR:
            case VIEW_3D_BOTTOM:
            case VIEW_3D_LEFT:
            case VIEW_3D_ROTATE_UP:
            case VIEW_3D_ROTATE_DOWN:
            case VIEW_3D_ROTATE_LEFT:
            case VIEW_3D_ROTATE_RIGHT:
            case VIEW_3D_PAN_UP:
            case VIEW_3D_PAN_DOWN:
            case VIEW_3D_PAN_LEFT:
            case VIEW_3D_PAN_RIGHT:
            case UNHIDE_EVERYTHING:
                return hasActiveEditor();
            case CUT:
            case COPY:
            case SELECT_NOTHING:
            case DELETE_SELECTED:
            case HIDE_SELECTED:
            case SELECT_CONNECTED:
            case INLINE_SELECTED_ELEMENTS:
            case START_TRANSLATING_SELECTED_NODES:
            case START_ROTATING_SELECTED_NODES:
            case START_MOVING_SELECTED_NODES:
                return hasSelectedNodes();
            case TRANSFORMATION_LOCK_X:
            case TRANSFORMATION_LOCK_Y:
            case TRANSFORMATION_LOCK_Z:
            case TRANSFORMATION_LOCK_XY:
            case TRANSFORMATION_LOCK_XZ:
            case TRANSFORMATION_LOCK_YZ:
            case END_TRANSFORMATION:
            case CANCEL_TRANSFORMATION:
                return hasTransformInProgress();
            default:
                return true;
        }
    }

    void executeUnchecked(Action action, std::shared_ptr<Editor> editorContext) {
        if (editorContext == nullptr) {
            editorContext = controller::getActiveEditor();
        }
        switch (action) {
            case DO_NOTHING:
                break;
            case EXIT:
                controller::setUserWantsToExit(true);
                break;
            case OPEN_FILE:
                gui::dialogs::showOpenFileDialog();
                break;
            case SAVE_FILE:
                editorContext->save();
                break;
            case SAVE_FILE_AS:
                gui::dialogs::showSaveFileAsDialog(editorContext);
                break;
            case SAVE_COPY_AS:
                gui::dialogs::showSaveCopyAsDialog(editorContext);
                break;
            case NEW_FILE:
                controller::createNewFile();
                break;
            case UNDO:
                controller::undoLastAction();
                break;
            case REDO:
                controller::redoLastAction();
                break;
            case CUT:
                controller::cutSelectedObject();
                break;
            case COPY:
                controller::copySelectedObject();
                break;
            case PASTE:
                controller::pasteObject();
                break;
            case SELECT_ALL:
                editorContext->nodeSelectAll();
                break;
            case SELECT_NOTHING:
                editorContext->nodeSelectNone();
                break;
            case VIEW_3D_FRONT:
                editorContext->setStandard3dView(1);
                break;
            case VIEW_3D_TOP:
                editorContext->setStandard3dView(2);
                break;
            case VIEW_3D_RIGHT:
                editorContext->setStandard3dView(3);
                break;
            case VIEW_3D_REAR:
                editorContext->setStandard3dView(4);
                break;
            case VIEW_3D_BOTTOM:
                editorContext->setStandard3dView(5);
                break;
            case VIEW_3D_LEFT:
                editorContext->setStandard3dView(6);
                break;
            case VIEW_3D_ROTATE_UP:
                editorContext->rotateViewUp();
                break;
            case VIEW_3D_ROTATE_DOWN:
                editorContext->rotateViewDown();
                break;
            case VIEW_3D_ROTATE_LEFT:
                editorContext->rotateViewLeft();
                break;
            case VIEW_3D_ROTATE_RIGHT:
                editorContext->rotateViewRight();
                break;
            case VIEW_3D_PAN_UP:
                editorContext->panViewUp();
                break;
            case VIEW_3D_PAN_DOWN:
                editorContext->panViewDown();
                break;
            case VIEW_3D_PAN_LEFT:
                editorContext->panViewLeft();
                break;
            case VIEW_3D_PAN_RIGHT:
                editorContext->panViewRight();
                break;
            case DELETE_SELECTED:
                editorContext->deleteSelectedElements();
                break;
            case HIDE_SELECTED:
                editorContext->hideSelectedElements();
                break;
            case UNHIDE_EVERYTHING:
                editorContext->unhideAllElements();
                break;
            case APPLY_DEFAULT_WINDOW_LAYOUT:
                gui::applyDefaultWindowLayout();
                break;
            case TAKE_SCREENSHOT:
                gui::dialogs::showScreenshotDialog(editorContext);
                break;
            case EXECUTE_ACTION_BY_NAME:
                gui::dialogs::showExecuteActionByNameDialog();
                break;
            case TOGGLE_TRANSFORM_GIZMO_ROTATION:
                controller::toggleTransformGizmoRotationState();
                break;
            case SELECT_CONNECTED:
                editorContext->nodeSelectConnected();
                break;
            case INLINE_SELECTED_ELEMENTS:
                editorContext->inlineSelectedElements();
                break;
            case START_TRANSLATING_SELECTED_NODES:
                editorContext->startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType::TRANSLATE);
                break;
            case START_ROTATING_SELECTED_NODES:
                editorContext->startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType::ROTATE);
                break;
            case START_MOVING_SELECTED_NODES:
                editorContext->startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType::MOVE);
                break;
            case TRANSFORMATION_LOCK_X:
                editorContext->getCurrentTransformAction()->toggleAxisLock(true, false, false);
                break;
            case TRANSFORMATION_LOCK_Y:
                editorContext->getCurrentTransformAction()->toggleAxisLock(false, true, false);
                break;
            case TRANSFORMATION_LOCK_Z:
                editorContext->getCurrentTransformAction()->toggleAxisLock(false, false, true);
                break;
            case TRANSFORMATION_LOCK_XY:
                editorContext->getCurrentTransformAction()->toggleAxisLock(true, true, false);
                break;
            case TRANSFORMATION_LOCK_XZ:
                editorContext->getCurrentTransformAction()->toggleAxisLock(true, false, true);
                break;
            case TRANSFORMATION_LOCK_YZ:
                editorContext->getCurrentTransformAction()->toggleAxisLock(false, true, true);
                break;
            case END_TRANSFORMATION:
                editorContext->endNodeTransformation();
                break;
            case CANCEL_TRANSFORMATION:
                editorContext->cancelNodeTransformation();
                break;
            default:
                spdlog::error("No case defined in user_actions::executeUnchecked for {}", magic_enum::enum_name(action));
                break;
        }
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
            magic_enum::enum_for_each<Action>([&name](auto action) {
                if (isInFilter(action, name)) {
                    results.push_back(action);
                }
            });
            lastName = name;
        }
        return results;
    }

    constexpr size_t getCount() {
        return actionsCount;
    }

    bool isInFilter(Action action, const std::string& filter) {
        //todo make this typo-robust
        return stringutil::containsIgnoreCase(getName(action), filter);
    }

    std::string_view getName(const Action& action) {
        #ifdef NDEBUG
        return names[action];
        #else
        assert(names[action].action == action);
        return names[action].name;
        #endif
    }

    void init() {
        ALL_ACTIONS.reserve(getCount());
        magic_enum::enum_for_each<Action>([](const auto& a) { ALL_ACTIONS.push_back(a); });
    }
}
