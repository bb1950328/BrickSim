// controller.h
// Created by bb1950328 on 09.10.20.
//

#ifndef BRICKSIM_CONTROLLER_H
#define BRICKSIM_CONTROLLER_H

#include "element_tree.h"
#include "renderer.h"
#include "gui.h"
#include "thumbnail_generator.h"

class Controller {
public:
    GLFWwindow *window;
    etree::ElementTree elementTree;
    bool elementTreeChanged = false;
    Renderer renderer;
    Gui gui;
    ThumbnailGenerator thumbnailGenerator;
    unsigned int view3dWidth = 800;
    unsigned int view3dHeight = 600;
    unsigned int windowWidth = config::get_long(config::SCREEN_WIDTH);
    unsigned int windowHeight = config::get_long(config::SCREEN_HEIGHT);
    long lastFrameTime = 0;//in µs
    bool userWantsToExit = false;
    std::set<etree::Node *> selectedNodes;
    int run();
    static Controller* getInstance();
    void set3dViewSize(unsigned int width, unsigned int height);
    void setWindowSize(unsigned int width, unsigned int height);

    void openFile(const std::string& path);

    void nodeSelectAddRemove(etree::Node *node);
    void nodeSelectUntil(etree::Node *node);
    void nodeSelectSet(etree::Node *node);
    void nodeSelectAll();
    void nodeSelectNone();

    void setStandard3dView(int i);

    void insertLdrElement(LdrFile* ldrFile);
    void deleteElement(etree::Node *nodeToDelete);
    void deleteSelectedElements();
private:
    Controller();
    //todo convert to namespace
    bool initializeGL();

    void runNormal();

    [[nodiscard]] bool doesUserWantToExit() const;
    etree::Node *currentlyEditingNode;
};

#endif //BRICKSIM_CONTROLLER_H
