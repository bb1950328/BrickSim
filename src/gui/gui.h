//
// Created by bb1950328 on 09.10.2020.
//

#ifndef BRICKSIM_GUI_H
#define BRICKSIM_GUI_H


#include "../ldr_files.h"
#include "../element_tree.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include "../lib/IconFontCppHeaders/IconsFontAwesome5.h"

constexpr int NUM_LDR_FILTER_PATTERNS = 4;
constexpr int NUM_IMAGE_FILTER_PATTERNS = 4;
constexpr int NUM_ZIP_FILTER_PATTERNS = 1;

namespace gui {
    namespace windows {
        void draw3dWindow(bool* show);
        void drawElementTreeWindow(bool* show);
        void drawElementPropertiesWindow(bool* show);
        void drawPartPaletteWindow(bool* show);
        void drawSettingsWindow(bool* show);
        void drawAboutWindow(bool* show);
        void drawSysInfoWindow(bool* show);
        void drawDebugWindow(bool* show);
        void drawOrientationCube(bool *show);
        void drawLogWindow(bool *show);
    }
    void drawMenuBar(bool* show);

    void setWindow(GLFWwindow* value);
    GLFWwindow* getWindow();
    void setLastScrollDeltaY(double value);
    double getLastScrollDeltaY();
    void setup();
    void setupStyle();
    void setupFont(float scaleFactor, ImGuiIO &io);
    [[nodiscard]] bool isSetupDone();

    void beginFrame();
    void drawMainWindows();
    void endFrame();

    void cleanup();

    enum PartsLibrarySetupResponse {
        RUNNING,//the function should be called again next time
        FINISHED,//installation is finsi
        REQUEST_EXIT//the user wants to exit the application
    };
    /**
     * @return true -> finished, false -> call this again in the next frame
     */
    PartsLibrarySetupResponse drawPartsLibrarySetupScreen();

    void drawWaitMessage(const std::string &message, float progress);
    void updateBlockingMessage(const std::string &message, float progress);
    void closeBlockingMessage();
};

#endif //BRICKSIM_GUI_H
