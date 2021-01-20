// controller.h
// Created by bb1950328 on 09.10.20.
//

#ifndef BRICKSIM_CONTROLLER_H
#define BRICKSIM_CONTROLLER_H

#include <mutex>
#include <atomic>
#include "element_tree.h"
#include "renderer.h"
#include "gui/gui.h"
#include "thumbnail_generator.h"
#include "tasks.h"

namespace controller {
    namespace {
        bool initializeGL();
        void window_size_callback(GLFWwindow *window, int width, int height);
        void checkForFinishedBackgroundTasks();
        void glfwErrorCallback(int code, const char* message);
        void initialize();
    }

    int run();
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

    void setElementTreeChanged(bool val);
    void setUserWantsToExit(bool val);
    std::set<etree::Node *>& getSelectedNodes();
    Renderer* getRenderer();
    etree::ElementTree& getElementTree();
    ThumbnailGenerator& getThumbnailGenerator();
    long getLastFrameTime();
    std::recursive_mutex & getOpenGlMutex();

    std::map<unsigned int, Task *> & getBackgroundTasks();
    void addBackgroundTask(std::string name, const std::function<void()>& function);

    std::queue<Task *>& getForegroundTasks();

    [[nodiscard]] bool doesUserWantToExit();
};

#endif //BRICKSIM_CONTROLLER_H
