#pragma once
#include "../editor/editor.h"
#include "GLFW/glfw3.h"

namespace bricksim::gui::dialogs {
    void drawDialogs(GLFWwindow* window);

    void showOpenFileDialog();
    void showSaveFileAsDialog();
    void showSaveFileAsDialog(const std::shared_ptr<Editor>& editor);
    void showSaveCopyAsDialog();
    void showSaveCopyAsDialog(const std::shared_ptr<Editor>& editor);
    void showScreenshotDialog();
    void showScreenshotDialog(const std::shared_ptr<Editor>& editor);
    void showExecuteActionByNameDialog();
}