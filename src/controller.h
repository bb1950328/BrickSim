

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
        void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
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
    void saveFile();
    void saveFileAs(const std::string& path);
    void saveCopyAs(const std::string& path);
    void createNewFile();

    void undoLastAction();
    void redoLastAction();

    void cutSelectedObject();
    void copySelectedObject();
    void pasteObject();

    void nodeSelectAddRemove(const std::shared_ptr<etree::Node>& node);
    void nodeSelectUntil(const std::shared_ptr<etree::Node>& node);
    void nodeSelectSet(const std::shared_ptr<etree::Node>& node);
    void nodeSelectAll();
    void nodeSelectNone();

    void setStandard3dView(int i);
    void rotateViewUp();
    void rotateViewDown();
    void rotateViewLeft();
    void rotateViewRight();
    void panViewUp();
    void panViewDown();
    void panViewLeft();
    void panViewRight();

    void insertLdrElement(const std::shared_ptr<LdrFile>& ldrFile);
    void deleteElement(const std::shared_ptr<etree::Node>& nodeToDelete);

    void deleteSelectedElements();
    void hideSelectedElements();
    void unhideAllElements();

    void setElementTreeChanged(bool val);
    void setUserWantsToExit(bool val);
    std::set<std::shared_ptr<etree::Node>> & getSelectedNodes();
    std::shared_ptr<Renderer> getRenderer();
    std::shared_ptr<etree::RootNode> getElementTree();
    std::shared_ptr<ThumbnailGenerator> getThumbnailGenerator();
    std::recursive_mutex & getOpenGlMutex();

    std::map<unsigned int, Task *> & getBackgroundTasks();
    void addBackgroundTask(std::string name, const std::function<void()>& function);

    std::queue<Task *>& getForegroundTasks();

    [[nodiscard]] bool doesUserWantToExit();

    std::tuple<unsigned short, float *, unsigned short> getLastFrameTimes();
};

#endif //BRICKSIM_CONTROLLER_H
