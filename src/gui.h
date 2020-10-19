//
// Created by bb1950328 on 09.10.2020.
//

#ifndef BRICKSIM_GUI_H
#define BRICKSIM_GUI_H


static const int NUM_LDR_FILTER_PATTERNS = 3;

static const int NUM_IMAGE_FILTER_PATTERNS = 4;

class Gui {
public:
    GLFWwindow *window;
    Gui() = default;
    void setup();
    void loop();
    void cleanup();

    double lastScrollDeltaY;

    bool loopPartsLibraryInstallationScreen();//returns true when finished

private:
    char const * lFilterPatterns[NUM_LDR_FILTER_PATTERNS] = {"*.ldr", "*.dat", "*.mpd"};
    char const * imageFilterPatterns[NUM_IMAGE_FILTER_PATTERNS] = {"*.png", "*.jpg", "*.bmp", "*.tga"};
    bool setupDone = false;
};

#endif //BRICKSIM_GUI_H
