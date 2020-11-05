// controller.h
// Created by bb1950328 on 09.10.20.
//

#ifndef BRICKSIM_CONTROLLER_H
#define BRICKSIM_CONTROLLER_H

#include <mutex>
#include "element_tree.h"
#include "renderer.h"
#include "gui.h"
#include "thumbnail_generator.h"
#include "background_task.h"

namespace controller {
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
    Gui& getGui();
    ThumbnailGenerator& getThumbnailGenerator();
    long getLastFrameTime();
    std::recursive_mutex & getOpenGlMutex();

    std::map<unsigned int, BackgroundTask *> & getBackgroundTasks();
    void addBackgroundTask(std::string name, const std::function<void()>& function);

    void runNormal();
    [[nodiscard]] bool doesUserWantToExit();
};

#endif //BRICKSIM_CONTROLLER_H
