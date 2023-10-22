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
    std::optional<std::filesystem::path> showSaveImageDialog(std::string title);
    std::optional<std::filesystem::path> showSaveDotFileDialog(std::string title);
    void showExecuteActionByNameDialog();

    void showError(const std::string& error);
}
