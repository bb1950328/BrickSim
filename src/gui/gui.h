#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <magic_enum.hpp>
#include <string>

constexpr int NUM_LDR_FILTER_PATTERNS = 4;
constexpr int NUM_IMAGE_FILTER_PATTERNS = 4;
constexpr int NUM_ZIP_FILTER_PATTERNS = 1;

namespace bricksim::gui {
    namespace {
        void setupStyle();
        void setupFont(float scaleFactor, ImGuiIO& io);
    }

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
    void showSaveCopyAsDialog();
    void showScreenshotDialog();
    void showExecuteActionByNameDialog();

    void applyDefaultWindowLayout();

    void cleanup();

    enum PartsLibrarySetupResponse {
        RUNNING,    //the function should be called again next time
        FINISHED,   //installation is finsi
        REQUEST_EXIT//the user wants to exit the application
    };

    PartsLibrarySetupResponse drawPartsLibrarySetupScreen();

    void drawWaitMessage(const std::string& message, float progress);
    void updateBlockingMessage(const std::string& message, float progress);
    void closeBlockingMessage();
};
