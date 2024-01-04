#pragma once

#include "editor/editor.h"
#include "element_tree.h"
#include "graphics/thumbnail_generator.h"
#include "ldr/files.h"
#include "snapping/snap_handler.h"
#include "tasks.h"
#include "types.h"
#include <GLFW/glfw3.h>
#include <efsw/efsw.hpp>
#include <functional>
#include <memory>
#include <queue>
#include <string>

#ifdef BRICKSIM_USE_RENDERDOC
    #include <renderdoc_app.h>
#endif

namespace bricksim::controller {
    namespace {
        bool initializeGL();
        void windowSizeCallback(GLFWwindow* window, int width, int height);
        void dropCallback(GLFWwindow* _, int count, const char** paths);
        void setWindowSize(unsigned int width, unsigned int height);
        void keyCallback(GLFWwindow* _, int key, int scancode, int action, int mods);
        void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        void checkForFinishedBackgroundTasks();
        void glfwErrorCallback(int code, const char* message);
        bool initialize();
        void cleanup();
        bool loopPartsLibrarySetupPrompt();
        void handleForegroundTasks();
    }

    int run();

    void openFile(const std::filesystem::path& absPath);
    void createNewFile();

    void undoLastAction();
    void redoLastAction();

    void cutSelectedObject();
    void copySelectedObject();
    void pasteObject();

    void toggleTransformGizmoRotationState();

    void setUserWantsToExit(bool val);
    std::shared_ptr<graphics::ThumbnailGenerator> getThumbnailGenerator();

    std::list<std::shared_ptr<Editor>>& getEditors();
    std::shared_ptr<Editor>& getActiveEditor();
    snap::Handler& getSnapHandler();

    void executeOpenGL(std::function<void()> const& functor);

    uomap_t<unsigned int, Task>& getBackgroundTasks();
    void addBackgroundTask(std::string name, const std::function<void()>& function);

    std::queue<Task>& getForegroundTasks();

    [[nodiscard]] bool doesUserWantToExit();

    std::tuple<unsigned short, float*, unsigned short> getLastFrameTimes();

    #ifdef BRICKSIM_USE_RENDERDOC
    RENDERDOC_API_1_1_2* getRenderdocAPI();
    #endif
    std::optional<std::shared_ptr<Editor>> getEditorOfScene(scene_id_t sceneId);
    void setActiveEditor(const std::shared_ptr<Editor>& editor);

    std::shared_ptr<efsw::FileWatcher> getFileWatcher();
}
