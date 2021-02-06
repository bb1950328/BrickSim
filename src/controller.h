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
        void cleanup();
        void loopPartsLibrarySetupPrompt();
    }

    int run();
    void set3dViewSize(unsigned int width, unsigned int height);

    void setWindowSize(unsigned int width, unsigned int height);

    void openFile(const std::string& path);
    void nodeSelectAddRemove(const std::shared_ptr<etree::Node>& node);
    void nodeSelectUntil(const std::shared_ptr<etree::Node>& node);
    void nodeSelectSet(const std::shared_ptr<etree::Node>& node);
    void nodeSelectAll();

    void nodeSelectNone();

    void setStandard3dView(int i);
    void insertLdrElement(const std::shared_ptr<LdrFile>& ldrFile);
    void deleteElement(const std::shared_ptr<etree::Node>& nodeToDelete);

    void deleteSelectedElements();

    void setElementTreeChanged(bool val);
    void setUserWantsToExit(bool val);
    std::set<std::shared_ptr<etree::Node>> & getSelectedNodes();
    std::shared_ptr<Renderer> getRenderer();
    std::shared_ptr<etree::ElementTree> getElementTree();
    std::shared_ptr<ThumbnailGenerator> getThumbnailGenerator();
    std::recursive_mutex & getOpenGlMutex();

    std::map<unsigned int, Task *> & getBackgroundTasks();
    void addBackgroundTask(std::string name, const std::function<void()>& function);

    std::queue<Task *>& getForegroundTasks();

    [[nodiscard]] bool doesUserWantToExit();

    std::tuple<unsigned short, float *, unsigned short> getLastFrameTimes();
};

#endif //BRICKSIM_CONTROLLER_H
