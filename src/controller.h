// controller.h
// Created by bab21 on 09.10.20.
//

#ifndef BRICKSIM_CONTROLLER_H
#define BRICKSIM_CONTROLLER_H

#include "element_tree.h"
#include "renderer.h"
#include "gui.h"

class Controller {
public:
    GLFWwindow *window;
    ElementTree elementTree;
    Renderer renderer;
    Gui gui;
    unsigned int windowWidth = Configuration::getInstance()->get_long(config::KEY_SCREEN_WIDTH);
    unsigned int windowHeight = Configuration::getInstance()->get_long(config::KEY_SCREEN_HEIGHT);
    int run();
    static Controller* getInstance();
    void setWindowSize(unsigned int width, unsigned int height);

private:
    Controller();
    //todo google how to make singleton
    bool initializeGL();
};

#endif //BRICKSIM_CONTROLLER_H
