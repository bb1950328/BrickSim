//
// Created by Bader on 09.10.2020.
//

#ifndef BRICKSIM_GUI_H
#define BRICKSIM_GUI_H

#include <GLFW/glfw3.h>

class Gui {
public:
    GLFWwindow *window;
    Gui() = default;
    void setup();
    void loop();
    void cleanup();
};

#endif //BRICKSIM_GUI_H
