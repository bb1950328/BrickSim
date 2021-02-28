#include "controller.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "info_providers/price_guide_provider.h"
#include "db.h"
#include "lib/stb_image.h"
#include "lib/stb_image_write.h"
#include "info_providers/bricklink_constants_provider.h"
#include "latest_log_messages_tank.h"
#include "ldr_files/ldr_file_repo.h"
#include "keyboard_shortcut_manager.h"
#include "config.h"
#include "metrics.h"

namespace controller {
    namespace {
        GLFWwindow *window;
        std::shared_ptr<etree::RootNode> elementTree;
        bool elementTreeChanged = false;
        bool selectionChanged = false;
        std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;
        std::shared_ptr<Scene> mainScene;
        std::shared_ptr<CadCamera> camera;
        unsigned int view3dWidth = 800;
        unsigned int view3dHeight = 600;
        unsigned int windowWidth;
        unsigned int windowHeight;

        bool userWantsToExit = false;
        std::set<std::shared_ptr<etree::Node>> selectedNodes;
        std::shared_ptr<etree::Node> currentlyEditingNode;
        std::map<unsigned int, Task *> backgroundTasks;//todo smart pointer
        std::queue<Task *> foregroundTasks;

        std::chrono::milliseconds idle_sleep(25);

        constexpr unsigned short lastFrameTimesSize = 256;
        float lastFrameTimes[lastFrameTimesSize] = {0};//in ms
        unsigned short lastFrameTimesStartIdx = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastMainloopTimePoint;
        std::vector<std::pair<const char*, unsigned int>> mainloopTimePointsUsTmp;

        void openGlDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
        {
            if (id==1282) {
                //there's a to do for that in thumbnail_generator
                return;
            }
            spdlog::level::level_enum level;
            switch (severity) {
                case GL_DEBUG_SEVERITY_NOTIFICATION: level = spdlog::level::info; break;
                case GL_DEBUG_SEVERITY_LOW: level = spdlog::level::debug; break;
                case GL_DEBUG_SEVERITY_MEDIUM: level = spdlog::level::warn; break;
                case GL_DEBUG_SEVERITY_HIGH: level = spdlog::level::err; break;
                default: level = spdlog::level::info;
            }

            const char* sourceStr;
            switch (source) {
                case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "WINDOW_SYSTEM"; break;
                case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "SHADER_COMPILER"; break;
                case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "THIRD_PARTY"; break;
                case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "APPLICATION"; break;
                case GL_DEBUG_SOURCE_OTHER:
                default: sourceStr = "OTHER"; break;
            }

            const char* typeStr;
            switch (type) {
                case GL_DEBUG_TYPE_ERROR: typeStr = "ERROR"; break;
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "DEPRECATED_BEHAVIOR"; break;
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "UNDEFINED_BEHAVIOR"; break;
                case GL_DEBUG_TYPE_PORTABILITY: typeStr = "PORTABILITY"; break;
                case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "PERFORMANCE"; break;
                case GL_DEBUG_TYPE_MARKER: typeStr = "MARKER"; break;
                case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "PUSH_GROUP"; break;
                case GL_DEBUG_TYPE_POP_GROUP: typeStr = "POP_GROUP"; break;
                case GL_DEBUG_TYPE_OTHER:
                default: typeStr = "OTHER"; break;
            }

            spdlog::log(level, "OpenGL debug message: source={}, type={}, id={}: {}", sourceStr, typeStr, id, message);
        }

        bool initializeGL() {
            std::lock_guard<std::recursive_mutex> lg(getOpenGlMutex());
            const auto enableDebugOutput = config::getBool(config::ENABLE_GL_DEBUG_OUTPUT);
            glfwSetErrorCallback(glfwErrorCallback);
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, enableDebugOutput?3:2);
            glfwWindowHint(GLFW_SAMPLES, (int) (config::getInt(config::MSAA_SAMPLES)));
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            if (enableDebugOutput) {
                spdlog::debug("OpenGL debug context is enabled");
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
            }

#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

            //glfwWindowHint(GLFW_DECORATED, false);//removes the title bar
            window = glfwCreateWindow(windowWidth, windowHeight, "BrickSim", nullptr, nullptr);
            if (window == nullptr) {
                spdlog::critical("Failed to create GLFW window");
                glfwTerminate();
                return false;
            }
            glfwMakeContextCurrent(window);
            if (!config::getBool(config::ENABLE_VSYNC)) {
                glfwSwapInterval(0);
            }
            glfwSetFramebufferSizeCallback(window, window_size_callback);
            glfwSetScrollCallback(window, scroll_callback);
            glfwSetKeyCallback(window, keyCallback);

            if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
                std::cerr << "Failed to initialize GLAD" << std::endl;
                return false;
            }

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);

            GLint flags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(openGlDebugMessageCallback, nullptr);
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            }

            spdlog::info("OpenGL initialized");
            return true;
        }

        void window_size_callback(GLFWwindow *_, int width, int height) {
            if (windowWidth != width || windowHeight!=height) {
                windowWidth = width;
                windowHeight = height;
                std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
                glViewport(0, 0, width, height);
            }
        }

        void keyCallback(GLFWwindow *_, int key, int scancode, int action, int mods) {
            keyboard_shortcut_manager::shortcutPressed(key, action, mods, gui::areKeysCaptured());
        }

        void scroll_callback(GLFWwindow *_, double xoffset, double yoffset) {
            //todo use xoffset to do something, maybe pan?
            gui::setLastScrollDeltaY(yoffset);
            if (ImGui::GetIO().WantCaptureMouse) {
                return;
            }
            camera->moveForwardBackward((float)yoffset);
        }


        void checkForFinishedBackgroundTasks() {
            static double lastCheck = 0;
            auto now = glfwGetTime();
            if (now - lastCheck > 0.5) {
                for (auto iter = backgroundTasks.begin(); iter != backgroundTasks.end();) {
                    if (iter->second->isDone()) {
                        iter->second->joinThread();
                        delete iter->second;
                        iter = backgroundTasks.erase(iter);
                    } else {
                        ++iter;
                    }
                }
                lastCheck = now;
            }
        }

        void loopPartsLibrarySetupPrompt() {
            auto status = gui::PartsLibrarySetupResponse::RUNNING;
            while (status == gui::PartsLibrarySetupResponse::RUNNING
                   && !glfwWindowShouldClose(window)) {
                gui::beginFrame();
                status = gui::drawPartsLibrarySetupScreen();
                gui::endFrame();
                {
                    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                }
            }
            if (status==gui::PartsLibrarySetupResponse::REQUEST_EXIT || glfwWindowShouldClose(window)) {
                cleanup();
                throw std::invalid_argument("user requested exit in parts library setup screen");//todo make cleaner solution
            }
        }

        void initialize() {
            const std::shared_ptr<latest_log_messages_tank::Sink<std::mutex>> x(new latest_log_messages_tank::Sink<std::mutex>);
            spdlog::default_logger()->sinks().push_back(x);
            spdlog::set_level(spdlog::level::trace);

            db::initialize();

            windowWidth = config::getInt(config::SCREEN_WIDTH);
            windowHeight = config::getInt(config::SCREEN_HEIGHT);

            if (!initializeGL()) {
                spdlog::critical("failed to initialize OpenGL / glfw, exiting");
                return;
            }

            util::setStbiFlipVertically(true);

            elementTree = std::make_shared<etree::RootNode>();
            camera = std::make_shared<CadCamera>();
            mainScene = scenes::get(scenes::MAIN_SCENE_ID);
            mainScene->setCamera(camera);
            mainScene->setRootNode(elementTree);

            gui::setWindow(window);
            gui::initialize();

            while (!ldr_file_repo::checkLdrawLibraryLocation()) {
                loopPartsLibrarySetupPrompt();
            }

            Task steps[]{
                    {"load color definitions",          ldr_color_repo::initialize},
                    {"initialize file list",            [](float *progress) { ldr_file_repo::get().initialize(progress); }},
                    {"initialize price guide provider", price_guide_provider::initialize},
                    {"initialize thumbnail generator",  []() { thumbnailGenerator = std::make_shared<ThumbnailGenerator>(); }},
                    {"initialize BrickLink constants",  [](float *progress) { bricklink_constants_provider::initialize(progress); }},
                    {"initialize keyboard shortcuts",  keyboard_shortcut_manager::initialize},
                    //{"initialize orientation cube generator", orientation_cube::initialize},
            };
            for (auto &initStep : steps) {
                spdlog::info("Starting init step {}", initStep.getName());
                initStep.startThread();
                while (!initStep.isDone()) {
                    if (gui::isSetupDone()) {
                        gui::beginFrame();
                        gui::drawWaitMessage(initStep.getName(), initStep.getProgress());
                        gui::endFrame();

                        {
                            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
                            glfwSwapBuffers(window);
                            glfwPollEvents();
                        }
                    } else {
                        std::chrono::milliseconds sleepTime(16);
                        std::this_thread::sleep_for(sleepTime);
                    }
                }
                initStep.joinThread();
                spdlog::info("finished init step {}", initStep.getName());
            }

            //spdlog::log(spdlog::level::trace, "a trace message");
            //spdlog::log(spdlog::level::debug, "a debug message");
            //spdlog::log(spdlog::level::info, "a info message");
            //spdlog::log(spdlog::level::warn, "a warn message");
            //spdlog::log(spdlog::level::err, "a err message");
            //spdlog::log(spdlog::level::critical, "a critical message");
        }

        void cleanup() {
            ldr_file_repo::get().cleanup();
            auto &bgTasks = getBackgroundTasks();
            spdlog::info("waiting for {} background threads to finish...", bgTasks.size());
            for (auto &task : bgTasks) {
                task.second->joinThread();
            }
            spdlog::info("all background tasks finished, exiting now");
            gui::cleanup();
            spdlog::shutdown();
            glfwTerminate();
        }

        void handleForegroundTasks() {
            while (!foregroundTasks.empty()) {
                Task *&frontTask = foregroundTasks.front();
                if (!frontTask->isStarted()) {
                    frontTask->startThread();
                }
                if (frontTask->isDone()) {
                    frontTask->joinThread();
                    delete frontTask;
                    foregroundTasks.pop();
                } else {
                    return;
                }
            }
        }

        void glfwErrorCallback(int code, const char *message) {
            spdlog::error("GLFW Error: {} {}", code, message);
        }
        
        void copyMainloopTimePoints() {
            metrics::mainloopTimePointsUs = mainloopTimePointsUsTmp;
            mainloopTimePointsUsTmp.clear();
        }
        
        void addMainloopTimePoint(const char* name) {
            const auto now = std::chrono::high_resolution_clock::now();
            mainloopTimePointsUsTmp.emplace_back(name, std::chrono::duration_cast<std::chrono::microseconds>(now - lastMainloopTimePoint).count());
            lastMainloopTimePoint = now;
        }
    }

    int run() {
        spdlog::info("BrickSim started.");
        initialize();

        openFile("test_files/bricks_test.ldr");
        //openFile("~/Downloads/arocs.mpd");
        //openFile("3001.dat");

        while (!(glfwWindowShouldClose(window) || userWantsToExit)) {
            copyMainloopTimePoints();
            if (foregroundTasks.empty() && backgroundTasks.empty() && thumbnailGenerator->renderQueueEmpty() && glfwGetWindowAttrib(window, GLFW_FOCUSED) == 0) {
                std::this_thread::sleep_for(idle_sleep);
                addMainloopTimePoint("idle sleep");
                glfwPollEvents();
                addMainloopTimePoint("glfwPollEvents();");
                continue;
            }

            const auto loopStart = glfwGetTime();
            auto before = std::chrono::high_resolution_clock::now();

            if (elementTreeChanged || selectionChanged) {
                //todo mainScene->meshCollection->updateSelectionContainerBox();
                selectionChanged = false;
                elementTreeChanged = true;
                addMainloopTimePoint("meshCollection->updateSelectionContainerBox()");
            }
            if (elementTreeChanged) {
                mainScene->elementTreeChanged();
                elementTreeChanged = false;
                addMainloopTimePoint("renderer->elementTreeChanged()");
            }
            mainScene->updateImage();
            addMainloopTimePoint("mainScene->updateImage()");

            gui::beginFrame();
            addMainloopTimePoint("gui::beginFrame()");
            gui::drawMainWindows();
            addMainloopTimePoint("gui::drawMainWindows()");

            handleForegroundTasks();
            if (foregroundTasks.empty()) {
                gui::closeBlockingMessage();
            } else {
                gui::updateBlockingMessage(foregroundTasks.front()->getName(), foregroundTasks.front()->getProgress());
            }
            addMainloopTimePoint("handle foreground tasks");

            gui::endFrame();
            addMainloopTimePoint("gui::endFrame()");

            thumbnailGenerator->discardOldestImages(0);
            bool moreWork;
            do {
                moreWork = thumbnailGenerator->workOnRenderQueue();
            } while (glfwGetTime() - loopStart < 1.0 / 60 && moreWork);
            addMainloopTimePoint("work on thumbnails");
            auto after = std::chrono::high_resolution_clock::now();
            lastFrameTimes[lastFrameTimesStartIdx] = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count()/1000.0f;
            lastFrameTimesStartIdx = (lastFrameTimesStartIdx+1)%lastFrameTimesSize;
            addMainloopTimePoint("add frame time");
            glFinish();
            addMainloopTimePoint("glFinish");
            glfwSwapBuffers(window);
            addMainloopTimePoint("glfwSwapBuffers");
            glfwPollEvents();
            addMainloopTimePoint("glfwPollEvents");
        }
        config::setInt(config::SCREEN_WIDTH, windowWidth);
        config::setInt(config::SCREEN_HEIGHT, windowHeight);
        cleanup();
        return 0;
    }

    bool doesUserWantToExit() {
        return glfwWindowShouldClose(window) || userWantsToExit;
    }

    void set3dViewSize(unsigned int width, unsigned int height) {
        view3dWidth = width;
        view3dHeight = height;
    }

    void openFile(const std::string &path) {
        foregroundTasks.push(new Task(std::string("Open ") + path, [path]() {
            insertLdrElement(ldr_file_repo::get().getFile(path));
        }));
    }

    void saveFile() {
        //todo implement
    }

    void saveFileAs(const std::string &path) {
        //todo implement
    }

    void saveCopyAs(const std::string &path) {
        //todo implement
    }

    void createNewFile() {
        //todo implement
    }

    void undoLastAction() {
        //todo implement
    }

    void redoLastAction() {
        //todo implement
    }

    void cutSelectedObject(){
        //todo implement
    }

    void copySelectedObject(){
        //todo implement
    }

    void pasteObject(){
        //todo implement
    }


    void nodeSelectAddRemove(const std::shared_ptr<etree::Node>& node) {
        auto iterator = selectedNodes.find(node);
        node->selected = iterator == selectedNodes.end();
        if (node->selected) {
            selectedNodes.insert(node);
        } else {
            selectedNodes.erase(iterator);
        }
        selectionChanged = true;
    }

    void nodeSelectSet(const std::shared_ptr<etree::Node>& node) {
        for (const auto &selectedNode : selectedNodes) {
            selectedNode->selected = false;
        }
        selectedNodes.clear();
        node->selected = true;
        selectedNodes.insert(node);
        selectionChanged = true;
    }

    void nodeSelectUntil(const std::shared_ptr<etree::Node>& node) {
        auto rangeActive = false;
        auto keepGoing = true;
        const auto& parentChildren = node->parent.lock()->getChildren();
        for (auto iterator = parentChildren.rbegin();
             iterator != parentChildren.rend() && keepGoing;
             iterator++) {
            auto itNode = *iterator;
            if (itNode == node || itNode->selected) {
                if (rangeActive) {
                    keepGoing = false;
                } else {
                    rangeActive = true;
                }
            }
            if (rangeActive) {
                itNode->selected = true;
                selectedNodes.insert(itNode);
            }
        }
        selectionChanged = true;
    }

    void nodeSelectAll() {
        nodeSelectNone();
        elementTree->selected = true;
        selectedNodes.insert(elementTree);
        selectionChanged = true;
    }

    void nodeSelectNone() {
        for (const auto &node : selectedNodes) {
            node->selected = false;
        }
        selectedNodes.clear();
        selectionChanged = true;
    }

    void setStandard3dView(int i) {
        camera->setStandardView(i);
    }

    //todo test these values
    void rotateViewUp(){camera->mouseRotate(0, -1);}
    void rotateViewDown(){camera->mouseRotate(0, +1);}
    void rotateViewLeft(){camera->mouseRotate(-1, 0);}
    void rotateViewRight(){camera->mouseRotate(+1, 0);}
    void panViewUp(){camera->mousePan(0, -1);}
    void panViewDown(){camera->mousePan(0, +1);}
    void panViewLeft(){camera->mousePan(-1, 0);}
    void panViewRight(){camera->mousePan(+1, 0);}

    void insertLdrElement(const std::shared_ptr<LdrFile>& ldrFile) {
        auto currentlyEditingLdrNode = std::dynamic_pointer_cast<etree::LdrNode>(currentlyEditingNode);
        switch (ldrFile->metaInfo.type) {
            case MODEL:
                currentlyEditingLdrNode = std::make_shared<etree::MpdNode>(ldrFile, LdrColorReference{2}, elementTree);
                currentlyEditingNode = currentlyEditingLdrNode;
                currentlyEditingLdrNode->createChildNodes();
                elementTree->addChild(currentlyEditingNode);
                break;
            case MPD_SUBFILE:
                if (nullptr != currentlyEditingLdrNode) {
                    currentlyEditingLdrNode->addSubfileInstanceNode(ldrFile, {1});
                }
                break;
            case PART:
                if (nullptr != currentlyEditingLdrNode) {
                    currentlyEditingLdrNode->addChild(std::make_shared<etree::PartNode>(ldrFile, LdrColorReference{1}, currentlyEditingNode));
                }
                break;
            default: return;
        }
        elementTreeChanged = true;
    }

    void deleteElement(const std::shared_ptr<etree::Node>& nodeToDelete) {
        nodeToDelete->parent.lock()->deleteChild(nodeToDelete);
        selectedNodes.erase(nodeToDelete);
        elementTreeChanged = true;
        selectionChanged = true;
    }

    void deleteSelectedElements() {
        for (const auto &node : selectedNodes) {
            deleteElement(node);
        }
    }

    void hideSelectedElements() {
        for (const auto &node : selectedNodes) {
            node->visible = false;
        }
    }

    void unhideElementRecursively(const std::shared_ptr<etree::Node>& node) {
        node->visible = false;
        for (const auto &child : node->getChildren()) {
            unhideElementRecursively(child);
        }
    }

    void unhideAllElements() {
        unhideElementRecursively(elementTree);
    }

    void setElementTreeChanged(bool val) {
        elementTreeChanged = val;
    }

    void setUserWantsToExit(bool val) {
        userWantsToExit = val;
    }

    std::set<std::shared_ptr<etree::Node>> & getSelectedNodes() {
        return selectedNodes;
    }

    std::shared_ptr<etree::RootNode> getElementTree() {
        return elementTree;
    }

    std::shared_ptr<ThumbnailGenerator> getThumbnailGenerator() {
        return thumbnailGenerator;
    }

    std::recursive_mutex &getOpenGlMutex() {
        static std::recursive_mutex openGlMutex;
        return openGlMutex;
    }

    std::map<unsigned int, Task *> &getBackgroundTasks() {
        checkForFinishedBackgroundTasks();
        return backgroundTasks;
    }

    void addBackgroundTask(std::string name, const std::function<void()> &function) {
        static unsigned int sId = 0;
        unsigned int id = sId++;
        auto *task = new Task(std::move(name), function);
        backgroundTasks.insert(std::make_pair(id, task));
        task->startThread();
    }

    std::queue<Task *> &getForegroundTasks() {
        return foregroundTasks;
    }

    std::tuple<unsigned short, float*, unsigned short> getLastFrameTimes() {
        return std::make_tuple(lastFrameTimesSize, lastFrameTimes, lastFrameTimesStartIdx);
    }
}