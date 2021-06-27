#ifndef BRICKSIM_CONTROLLER_H
#define BRICKSIM_CONTROLLER_H

#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <functional>
#include <queue>
#include "element_tree.h"
#include "types.h"
#include "ldr_files/ldr_files.h"
#include "transform_gizmo.h"
#include "graphics/thumbnail_generator.h"
#include "tasks.h"

#ifdef BRICKSIM_USE_RENDERDOC
#include <renderdoc_app.h>
#endif

namespace bricksim::controller {
    namespace {
        bool initializeGL();
        void window_size_callback(GLFWwindow *window, int width, int height);
        void setWindowSize(unsigned int width, unsigned int height);
        void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
        void checkForFinishedBackgroundTasks();
        void glfwErrorCallback(int code, const char* message);
        bool initialize();
        void cleanup();
        void loopPartsLibrarySetupPrompt();
    }

    int run();
    void set3dViewSize(unsigned int width, unsigned int height);

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

    bool isNodeClickable(const std::shared_ptr<etree::Node>& node);
    void nodeClicked(const std::shared_ptr<etree::Node>& clickedNode, bool ctrlPressed, bool shiftPressed);

    bool isNodeDraggable(const std::shared_ptr<etree::Node>& node);
    void startNodeDrag(std::shared_ptr<etree::Node> &node, const glm::svec2 initialCursorPos);
    void updateNodeDragDelta(glm::usvec2 delta);
    void endNodeDrag();

    void setStandard3dView(int i);
    void rotateViewUp();
    void rotateViewDown();
    void rotateViewLeft();
    void rotateViewRight();
    void panViewUp();
    void panViewDown();
    void panViewLeft();
    void panViewRight();

    void insertLdrElement(const std::shared_ptr<ldr::File>& ldrFile);
    void deleteElement(const std::shared_ptr<etree::Node>& nodeToDelete);

    void deleteSelectedElements();
    void hideSelectedElements();
    void unhideAllElements();

    transform_gizmo::RotationState getTransformGizmoRotationState();
    void toggleTransformGizmoRotationState();

    void setElementTreeChanged(bool val);
    void setUserWantsToExit(bool val);
    std::set<std::shared_ptr<etree::Node>> & getSelectedNodes();
    std::shared_ptr<etree::RootNode> getElementTree();
    std::shared_ptr<graphics::ThumbnailGenerator> getThumbnailGenerator();
    std::shared_ptr<graphics::Scene> getMainScene();
    std::shared_ptr<graphics::CadCamera> getMainSceneCamera();

    void executeOpenGL(std::function<void()> const &functor);

    std::map<unsigned int, Task *> & getBackgroundTasks();
    void addBackgroundTask(std::string name, const std::function<void()>& function);

    std::queue<Task *>& getForegroundTasks();

    [[nodiscard]] bool doesUserWantToExit();

    std::tuple<unsigned short, float *, unsigned short> getLastFrameTimes();

#ifdef BRICKSIM_USE_RENDERDOC
    RENDERDOC_API_1_1_2 *getRenderdocAPI();
#endif
};

#endif //BRICKSIM_CONTROLLER_H
