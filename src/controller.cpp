// controller.cpp
// Created by bb1950328 on 09.10.20.
//

#include "controller.h"

#include <utility>
#include "info_providers/price_guide_provider.h"
#include "db.h"
#include "orientation_cube.h"
#include "tasks.h"
#include "lib/stb_image.h"
#include "info_providers/bricklink_constants_provider.h"

namespace controller {
    namespace {
        GLFWwindow *window;
        etree::ElementTree elementTree;
        bool elementTreeChanged = false;
        bool selectionChanged = false;
        Renderer renderer(&elementTree);
        ThumbnailGenerator thumbnailGenerator(&renderer);
        unsigned int view3dWidth = 800;
        unsigned int view3dHeight = 600;
        unsigned int windowWidth;
        unsigned int windowHeight;
        long lastFrameTime = 0;//in µs
        bool userWantsToExit = false;
        std::set<etree::Node *> selectedNodes;
        etree::Node *currentlyEditingNode;
        std::map<unsigned int, Task*> backgroundTasks;
        std::queue<Task *> foregroundTasks;

        std::chrono::milliseconds idle_sleep(25);

        bool initializeGL() {
            std::lock_guard<std::recursive_mutex> lg(getOpenGlMutex());
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_SAMPLES, (int) (config::getInt(config::MSAA_SAMPLES)));
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

            //glfwWindowHint(GLFW_DECORATED, false);//removes the title bar
            window = glfwCreateWindow(windowWidth, windowHeight, "BrickSim", nullptr, nullptr);
            if (window == nullptr) {
                std::cout << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return false;
            }
            glfwMakeContextCurrent(window);
            glfwSetFramebufferSizeCallback(window, window_size_callback);
            glfwSetScrollCallback(window, scroll_callback);

            if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
                std::cerr << "Failed to initialize GLAD" << std::endl;
                return false;
            }

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            std::cout << "INFO: OpenGL initialized" << std::endl;
            return true;
        }

        void window_size_callback(GLFWwindow *window, int width, int height) {
            setWindowSize(width, height);
        }

        void checkForFinishedBackgroundTasks() {
            static double lastCheck = 0;
            auto now = glfwGetTime();
            if (now-lastCheck>0.5) {
                for(auto iter = backgroundTasks.begin(); iter != backgroundTasks.end(); ) {
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

        void initialize() {
            db::initialize();

            windowWidth = config::getInt(config::SCREEN_WIDTH);
            windowHeight = config::getInt(config::SCREEN_HEIGHT);

            if (!initializeGL()) {
                std::cerr << "FATAL: failed to initialize OpenGL / glfw" << std::endl;
                return;
            }

            util::setStbiFlipVertically(true);

            renderer.window = window;
            gui::setWindow(window);
            gui::setup();
            renderer.setWindowSize(view3dWidth, view3dHeight);
            renderer.setup();

            Task steps[]{
                    {"load color definitions", [](){ldr_color_repo::initialize();}},
                    {"initialize file list", [](float *progress){ldr_file_repo::initializeFileList(progress);}},
                    {"initialize price guide provider", [](){price_guide_provider::initialize();}},
                    {"initialize thumbnail generator", [](){thumbnailGenerator.initialize();}},
                    {"initialize BrickLink constants", [](float *progress){ bricklink_constants_provider::initialize(progress);}},
                    //{"initialize orientation cube generator", [](){orientation_cube::initialize();}},
            };
            for (auto &initStep : steps) {
                std::cout << initStep.getName() << std::endl;
                auto before = std::chrono::high_resolution_clock::now();
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
                auto after = std::chrono::high_resolution_clock::now();
                std::cout << initStep.getName() << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count() << "ms." << std::endl;
            }
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
    }

    int run() {
        //todo check if parts library found (method in ldr_file_repo::)
        initialize();

        //openFile("test_files/mpd_test.mpd");
        openFile("~/Downloads/arocs.mpd");
        //openFile("3001.dat");

        while (!(glfwWindowShouldClose(window) || userWantsToExit)) {
            if (foregroundTasks.empty() && backgroundTasks.empty() && thumbnailGenerator.renderQueueEmpty() && glfwGetWindowAttrib(window, GLFW_FOCUSED)==0) {
                std::this_thread::sleep_for(idle_sleep);
                glfwPollEvents();
                continue;
            }

            const auto loopStart = glfwGetTime();
            auto before = std::chrono::high_resolution_clock::now();

            if (elementTreeChanged || selectionChanged) {
                renderer.meshCollection.updateSelectionContainerBox();
                selectionChanged = false;
                elementTreeChanged = true;
            }
            if (elementTreeChanged) {
                renderer.elementTreeChanged();
                elementTreeChanged = false;
            }
            renderer.loop();

            gui::beginFrame();
            gui::drawMainWindows();

            handleForegroundTasks();
            if (foregroundTasks.empty()) {
                gui::closeBlockingMessage();
            } else {
                gui::updateBlockingMessage(foregroundTasks.front()->getName(), foregroundTasks.front()->getProgress());
            }

            gui::endFrame();

            thumbnailGenerator.discardOldestImages(0);
            bool moreWork;
            do {
                moreWork = thumbnailGenerator.workOnRenderQueue();
            } while (glfwGetTime() - loopStart < 1.0 / 60 && moreWork);
            auto after = std::chrono::high_resolution_clock::now();
            lastFrameTime = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        config::setInt(config::SCREEN_WIDTH, windowWidth);
        config::setInt(config::SCREEN_HEIGHT, windowHeight);
        renderer.cleanup();
        auto &bgTasks = getBackgroundTasks();
        std::cout << "waiting for " << bgTasks.size() << " background tasks to finish..." << std::endl;
        for (auto &task : bgTasks) {
            task.second->joinThread();
        }
        std::cout << "all background tasks finished, exiting now" << std::endl;
        gui::cleanup();
        glfwTerminate();
        return 0;
    }

    bool doesUserWantToExit() {
        return glfwWindowShouldClose(window) || userWantsToExit;
    }

    void set3dViewSize(unsigned int width, unsigned int height) {
        view3dWidth = width;
        view3dHeight = height;
        renderer.setWindowSize(width, height);
    }

    void setWindowSize(unsigned int width, unsigned int height) {
        windowWidth = width;
        windowHeight = height;
    }

    void openFile(const std::string &path) {
        foregroundTasks.push(new Task(std::string("Open ")+path, [path](){
            insertLdrElement(ldr_file_repo::getFile(path));
        }));
    }

    void nodeSelectAddRemove(etree::Node *node) {
        auto iterator = selectedNodes.find(node);
        node->selected = iterator == selectedNodes.end();
        if (node->selected) {
            selectedNodes.insert(node);
        } else {
            selectedNodes.erase(iterator);
        }
        selectionChanged = true;
    }

    void nodeSelectSet(etree::Node *node) {
        for (const auto &selectedNode : selectedNodes) {
            selectedNode->selected = false;
        }
        selectedNodes.clear();
        node->selected = true;
        selectedNodes.insert(node);
        selectionChanged = true;
    }

    void nodeSelectUntil(etree::Node *node) {
        auto rangeActive = false;
        auto keepGoing = true;
        for (auto iterator = node->parent->getChildren().rbegin();
             iterator != node->parent->getChildren().rend() && keepGoing;
             iterator++) {
            etree::Node *itNode = *iterator;
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
        elementTree.rootNode.selected = true;
        selectedNodes.insert(&elementTree.rootNode);
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
        renderer.camera.setStandardView(i);
        renderer.unrenderedChanges = true;
    }

    void insertLdrElement(LdrFile *ldrFile) {
        auto currentlyEditingLdrNode = dynamic_cast<etree::LdrNode *>(currentlyEditingNode);
        switch (ldrFile->metaInfo.type) {
            case MODEL:currentlyEditingNode = new etree::MpdNode(ldrFile, ldr_color_repo::get_color(2), &elementTree.rootNode);
                elementTree.rootNode.addChild(currentlyEditingNode);
                break;
            case MPD_SUBFILE:
                if (nullptr != currentlyEditingLdrNode) {
                    currentlyEditingLdrNode->addSubfileInstanceNode(ldrFile, ldr_color_repo::get_color(1));
                }
                break;
            case PART:
                if (nullptr != currentlyEditingLdrNode) {
                    currentlyEditingLdrNode->addChild(new etree::PartNode(ldrFile, ldr_color_repo::get_color(1), currentlyEditingNode));
                }
                break;
            default: return;
        }
        elementTreeChanged = true;
    }

    void deleteElement(etree::Node *nodeToDelete) {
        nodeToDelete->parent->deleteChild(nodeToDelete);
        selectedNodes.erase(nodeToDelete);
        elementTreeChanged = true;
        selectionChanged = true;
    }

    void deleteSelectedElements() {
        for (const auto &node : selectedNodes) {
            deleteElement(node);
        }
    }

    void setElementTreeChanged(bool val) {
        elementTreeChanged = val;
    }

    void setUserWantsToExit(bool val) {
        userWantsToExit = val;
    }

    std::set<etree::Node *> &getSelectedNodes() {
        return selectedNodes;
    }

    Renderer *getRenderer() {
        return &renderer;
    }

    etree::ElementTree &getElementTree() {
        return elementTree;
    }

    ThumbnailGenerator &getThumbnailGenerator() {
        return thumbnailGenerator;
    }

    long getLastFrameTime() {
        return lastFrameTime;
    }

    std::recursive_mutex &getOpenGlMutex() {
        static std::recursive_mutex openGlMutex;
        return openGlMutex;
    }

    std::map<unsigned int, Task*> &getBackgroundTasks() {
        checkForFinishedBackgroundTasks();
        return backgroundTasks;
    }

    void addBackgroundTask(std::string name, const std::function<void()>& function) {
        static unsigned int sId = 0;
        unsigned int id = sId++;
        auto *task = new Task(std::move(name), function);
        backgroundTasks.insert(std::make_pair(id, task));
        task->startThread();
    }

    std::queue<Task *>& getForegroundTasks() {
        return foregroundTasks;
    }
}