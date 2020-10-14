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
    unsigned int view3dWidth = 800;//todo customizable
    unsigned int view3dHeight = 600;
    unsigned int windowWidth = config::get_long(config::KEY_SCREEN_WIDTH);
    unsigned int windowHeight = config::get_long(config::KEY_SCREEN_HEIGHT);
    long lastFrameTime = 0;//in µs
    bool userWantsToExit = false;
    int run();
    static Controller* getInstance();
    void set3dViewSize(unsigned int width, unsigned int height);
    void setWindowSize(unsigned int width, unsigned int height);

    void openFile(const std::string& path);

private:
    Controller();
    //todo google how to make singleton
    bool initializeGL();
};

#endif //BRICKSIM_CONTROLLER_H
