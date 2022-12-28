#pragma once

#include "../graphics/texture.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <magic_enum.hpp>
#include <string>

namespace bricksim {
    class Editor;
}

namespace bricksim::gui {
    namespace {
        void setupStyle();
        void setupFont(float scaleFactor, ImGuiIO& io);
    }

    struct UserQuestion {
        std::string title;
        std::string question;
        std::vector<std::string> options;
    };

    void drawMenuBar(bool* show);

    void setWindow(GLFWwindow* value);
    GLFWwindow* getWindow();

    void setLastScrollDeltaY(double value);
    double getLastScrollDeltaY();

    bool areKeysCaptured();

    void initialize();
    [[nodiscard]] bool isSetupDone();

    void beginFrame();
    void drawMainWindows();
    void endFrame();

    void showOpenFileDialog();
    void showSaveFileAsDialog();
    void showSaveFileAsDialog(const std::shared_ptr<Editor>& editor);
    void showSaveCopyAsDialog();
    void showSaveCopyAsDialog(const std::shared_ptr<Editor>& editor);
    void showScreenshotDialog();
    void showScreenshotDialog(const std::shared_ptr<Editor>& editor);
    void showExecuteActionByNameDialog();

    void drawDocumentMenu(const std::shared_ptr<Editor>& editor);

    void applyDefaultWindowLayout();

    void cleanup();

    enum class PartsLibrarySetupResponse {
        RUNNING,    //the function should be called again next time
        FINISHED,   //installation is finsi
        REQUEST_EXIT//the user wants to exit the application
    };

    PartsLibrarySetupResponse drawPartsLibrarySetupScreen();

    const std::shared_ptr<graphics::Texture>& getLogoTexture();

    void drawWaitMessage(const std::string& message, float progress);
}
