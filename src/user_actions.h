#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"


#ifndef BRICKSIM_USER_ACTIONS_H
#define BRICKSIM_USER_ACTIONS_H

#include <functional>
#include "controller.h"

namespace user_actions {
    struct Action {
        const int id;
        const char* name;
        const char* nameWithIcon;
        std::function<void()> function;
    };
    // add new user actions here and in user_actions.cpp
    // IMPORTANT: only append at the end, because otherwise the id's get messed up
    int aId=0;
    const Action EXIT{0, "Exit", ICON_FA_SIGN_OUT_ALT " Exit", [](){controller::setUserWantsToExit(true);}};
    const Action OPEN_FILE{1, "Open", ICON_FA_FOLDER_OPEN " Open", gui::showOpenFileDialog};
    const Action SAVE_FILE{2, "Save", ICON_FA_SAVE" Save", controller::saveFile};
    const Action SAVE_FILE_AS{3, "Save as", ICON_FA_SAVE" Save as", gui::showSaveFileAsDialog};
    const Action SAVE_COPY_AS{4, "Save copy as", ICON_FA_COPY" Save copy as", gui::showSaveCopyAsDialog};
    const Action NEW_FILE{5, "New file", ICON_FA_PLUS" New file", controller::createNewFile};
    const Action UNDO{6, "Undo", ICON_FA_UNDO" Undo", controller::undoLastAction};
    const Action REDO{7, "Redo", ICON_FA_REDO" Redo", controller::redoLastAction};
    const Action CUT{8, "Cut", ICON_FA_CUT" Cut", controller::cutSelectedObject};
    const Action COPY{9, "Copy", ICON_FA_COPY" Copy", controller::copySelectedObject};
    const Action PASTE{10, "Paste", ICON_FA_PASTE" Paste", controller::pasteObject};
    const Action SELECT_ALL{11, "Select all", ICON_FA_CHECK_SQUARE" Select all", controller::nodeSelectAll};
    const Action SELECT_NOTHING{12, "Select nothing", ICON_FA_MINUS_SQUARE" Select nothing", controller::nodeSelectNone};
    const Action VIEW_3D_FRONT{13, "Front view", ICON_FA_DICE_ONE" Front view", [](){controller::setStandard3dView(1);}};
    const Action VIEW_3D_TOP{14, "Top view", ICON_FA_DICE_TWO" Top view", [](){controller::setStandard3dView(2);}};
    const Action VIEW_3D_RIGHT{15, "Right view", ICON_FA_DICE_THREE" Right view", [](){controller::setStandard3dView(3);}};
    const Action VIEW_3D_REAR{16, "Rear view", ICON_FA_DICE_FOUR" Rear view", [](){controller::setStandard3dView(4);}};
    const Action VIEW_3D_BOTTOM{17, "Bottom view", ICON_FA_DICE_FIVE" Bottom view", [](){controller::setStandard3dView(5);}};
    const Action VIEW_3D_LEFT{18, "Left view", ICON_FA_DICE_SIX" Left view", [](){controller::setStandard3dView(6);}};
    const Action VIEW_3D_ROTATE_UP{19, "Rotate view upwards", ICON_FA_ANGLE_UP" Rotate view upwards", controller::rotateViewUp};
    const Action VIEW_3D_ROTATE_DOWN{20, "Rotate view downwards", ICON_FA_ANGLE_DOWN" Rotate view downwards", controller::rotateViewDown};
    const Action VIEW_3D_ROTATE_LEFT{21, "Rotate view left", ICON_FA_ANGLE_LEFT" Rotate view left", controller::rotateViewLeft};
    const Action VIEW_3D_ROTATE_RIGHT{22, "Rotate view right", ICON_FA_ANGLE_RIGHT" Rotate view right", controller::rotateViewRight};
    const Action VIEW_3D_PAN_UP{23, "Pan view upwards", ICON_FA_ANGLE_DOUBLE_UP" Pan view upwards", controller::panViewUp};
    const Action VIEW_3D_PAN_DOWN{24, "Pan view downwards", ICON_FA_ANGLE_DOUBLE_DOWN" Pan view downwards", controller::panViewDown};
    const Action VIEW_3D_PAN_LEFT{25, "Pan view left", ICON_FA_ANGLE_DOUBLE_LEFT" Pan view left", controller::panViewLeft};
    const Action VIEW_3D_PAN_RIGHT{26, "Pan view right", ICON_FA_ANGLE_DOUBLE_RIGHT" Pan view right", controller::panViewRight};
    const Action DELETE_SELECTED{27, "Delete selected element(s)", ICON_FA_TRASH_ALT" Delete selected element(s)", controller::deleteSelectedElements};
    const Action HIDE_SELECTED{28, "Hide selected element(s)", ICON_FA_EYE_SLASH" Hide selected element(s)", controller::hideSelectedElements};
    const Action UNHIDE_EVERYTHING{29, "Unhide all elements", ICON_FA_EYE" Unhide all elements", controller::unhideAllElements};

    void initialize();
    void executeAction(int id);
    std::vector<Action> & findActionsByName(const std::string& name);
    const std::vector<Action>& getAllActions();
}
#endif //BRICKSIM_USER_ACTIONS_H

#pragma clang diagnostic pop