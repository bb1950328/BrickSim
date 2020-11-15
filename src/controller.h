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
#include "background_task.h"

namespace controller {
    namespace {
        bool initializeGL();
        void window_size_callback(GLFWwindow *window, int width, int height);
        void checkForFinishedBackgroundTasks();
        void initialize();
        class InitialisationStep {
        private:
            std::string name;
            std::function<void()> task;
            std::thread* taskThread = nullptr;
            std::atomic<bool> taskFinished = false;
        public:
            InitialisationStep(std::string name, std::function<void()> task);
            void start();
            bool isFinished();
            [[nodiscard]] const std::string &getName() const;
        };
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

    std::map<unsigned int, BackgroundTask *> & getBackgroundTasks();
    void addBackgroundTask(std::string name, const std::function<void()>& function);

    [[nodiscard]] bool doesUserWantToExit();
};

#endif //BRICKSIM_CONTROLLER_H
