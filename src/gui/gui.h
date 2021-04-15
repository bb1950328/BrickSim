#ifndef BRICKSIM_GUI_H
#define BRICKSIM_GUI_H


#include "../ldr_files/ldr_files.h"
#include "../element_tree.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include "../lib/IconFontCppHeaders/IconsFontAwesome5.h"

constexpr int NUM_LDR_FILTER_PATTERNS = 4;
constexpr int NUM_IMAGE_FILTER_PATTERNS = 4;
constexpr int NUM_ZIP_FILTER_PATTERNS = 1;

namespace gui {
    extern const char *WINDOW_NAME_3D_VIEW;
    extern const char *WINDOW_NAME_ELEMENT_TREE;
    extern const char *WINDOW_NAME_ELEMENT_PROPERTIES;
    extern const char *WINDOW_NAME_PART_PALETTE;
    extern const char *WINDOW_NAME_SETTINGS;
    extern const char *WINDOW_NAME_ABOUT;
    extern const char *WINDOW_NAME_SYSTEM_INFO;
    extern const char *WINDOW_NAME_DEBUG;
    extern const char *WINDOW_NAME_IMGUI_DEMO;
    extern const char *WINDOW_NAME_ORIENTATION_CUBE;
    extern const char *WINDOW_NAME_LOG;
    extern const char *WINDOW_NAME_GEAR_RATIO_CALCULATOR;

    namespace windows {
        void draw3dWindow(bool *show);
        void drawElementTreeWindow(bool *show);
        void drawElementPropertiesWindow(bool *show);
        void drawPartPaletteWindow(bool *show);
        void drawSettingsWindow(bool *show);
        void drawAboutWindow(bool *show);
        void drawSysInfoWindow(bool *show);
        void drawDebugWindow(bool *show);
        void drawOrientationCube(bool *show);
        void drawLogWindow(bool *show);
        void drawGearRatioCalculatorWindow(bool *show);
    }

    namespace {
        void setupStyle();
        void setupFont(float scaleFactor, ImGuiIO &io);
    }

    void drawMenuBar(bool *show);

    void setWindow(GLFWwindow *value);
    GLFWwindow *getWindow();

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
        RUNNING,//the function should be called again next time
        FINISHED,//installation is finsi
        REQUEST_EXIT//the user wants to exit the application
    };

    PartsLibrarySetupResponse drawPartsLibrarySetupScreen();

    void drawWaitMessage(const std::string &message, float progress);
    void updateBlockingMessage(const std::string &message, float progress);
    void closeBlockingMessage();
};

#endif //BRICKSIM_GUI_H
