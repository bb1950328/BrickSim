//
// Created by Bader on 09.10.2020.
//

#ifndef BRICKSIM_GUI_H
#define BRICKSIM_GUI_H


class Gui {
public:
    GLFWwindow *window;
    Gui() = default;
    void setup();
    void loop();
    void cleanup();

    double lastScrollDeltaY;
private:
    char const * lFilterPatterns[3] = { "*.ldr", "*.dat", "*.mpd"};
};

#endif //BRICKSIM_GUI_H
