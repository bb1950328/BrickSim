//
// Created by bb1950328 on 09.10.2020.
//

#ifndef BRICKSIM_GUI_H
#define BRICKSIM_GUI_H


#include "../ldr_files.h"
#include "../element_tree.h"
#include <imgui.h>
#include <GLFW/glfw3.h>

constexpr int NUM_LDR_FILTER_PATTERNS = 4;

constexpr int NUM_IMAGE_FILTER_PATTERNS = 4;

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
    }
    void drawMenuBar(bool* show);

    void setWindow(GLFWwindow* value);
    GLFWwindow* getWindow();
    void setLastScrollDeltaY(double value);
    double getLastScrollDeltaY();
    void setup();
    [[nodiscard]] bool isSetupDone();
    void loop();

    void cleanup();

    bool loopPartsLibraryInstallationScreen();//returns true when finished

    void drawWaitMessage(const std::string& message);
};

#endif //BRICKSIM_GUI_H
