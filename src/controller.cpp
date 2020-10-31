// controller.cpp
// Created by bb1950328 on 09.10.20.
//

#include "controller.h"

void window_size_callback(GLFWwindow *window, int width, int height);

int Controller::run() {
    if (!initializeGL()) {
        std::cerr << "FATAL: failed to initialize OpenGL / glfw" << std::endl;
        return -1;
    }
    renderer.window = window;
    gui.window = window;
    //openFile("test_files/mpd_test.mpd");
    openFile("~/Downloads/arocs.mpd");
    //openFile("3001.dat");
    gui.setup();
    bool partsLibraryFound = false;
    while (!partsLibraryFound && !doesUserWantToExit()) {
        partsLibraryFound = ldr_file_repo::initializeNames();
        while (!partsLibraryFound && !doesUserWantToExit()) {
            bool installFinished = gui.loopPartsLibraryInstallationScreen();
            glfwSwapBuffers(window);
            glfwPollEvents();
            if (installFinished) {
                break;
            }
        }
    }
    if (partsLibraryFound) {
        /*for (const auto &category : ldr_file_repo::getPartsGroupedByCategory()) {
            std::cout << category.first << ": " << category.second.size()  << " (first:" << (*category.second.begin())->metaInfo.name << ")" << std::endl;
        }*/
        runNormal();
    }
    gui.cleanup();
    glfwTerminate();
    return 0;
}

bool Controller::doesUserWantToExit() const { return glfwWindowShouldClose(window) || userWantsToExit; }

void Controller::runNormal() {
    renderer.setWindowSize(view3dWidth, view3dHeight);
    renderer.setup();
    while (!(glfwWindowShouldClose(window) || userWantsToExit)) {
        auto before = std::chrono::high_resolution_clock::now();
        if (elementTreeChanged) {
            renderer.elementTreeChanged();
            elementTreeChanged = false;
        }
        renderer.loop();
        gui.loop();
        auto after = std::chrono::high_resolution_clock::now();
        lastFrameTime = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    config::set_long(config::SCREEN_WIDTH, windowWidth);
    config::set_long(config::SCREEN_HEIGHT, windowHeight);
    config::save();
    renderer.cleanup();
}

Controller *Controller::getInstance() {
    static Controller instance;
    return &instance;
}

Controller::Controller() : renderer(&elementTree), thumbnailGenerator(&renderer) {

}

bool Controller::initializeGL() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, (int)(config::get_long(config::MSAA_SAMPLES)));
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //glfwWindowHint(GLFW_DECORATED, false);//removes the title bar
    window = glfwCreateWindow(windowWidth, windowHeight, "BrickSim", nullptr, nullptr);
    //glfwSetWindowPos(window, 20, 40);
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
    return true;
}

void Controller::set3dViewSize(unsigned int width, unsigned int height) {
    view3dWidth = width;
    view3dHeight = height;
    renderer.setWindowSize(width, height);
}

void Controller::setWindowSize(unsigned int width, unsigned int height) {
    windowWidth = width;
    windowHeight = height;
}

void Controller::openFile(const std::string& path) {
    elementTree.loadLdrFile(path);
    elementTreeChanged = true;
}

void Controller::nodeSelectAddRemove(etree::Node *node) {
    auto iterator = selectedNodes.find(node);
    node->selected = iterator == selectedNodes.end();
    if (node->selected) {
        selectedNodes.insert(node);
    } else {
        selectedNodes.erase(iterator);
    }
}

void Controller::nodeSelectSet(etree::Node *node) {
    for (const auto &selectedNode : selectedNodes) {
        selectedNode->selected = false;
    }
    selectedNodes.clear();
    node->selected = true;
    selectedNodes.insert(node);
}

void Controller::nodeSelectUntil(etree::Node *node) {
    auto rangeActive = false;
    auto keepGoing = true;
    for (auto iterator = node->parent->getChildren().rbegin();
         iterator!=node->parent->getChildren().rend() && keepGoing;
         iterator++) {
        etree::Node *itNode = *iterator;
        if (itNode==node || itNode->selected) {
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
}

void Controller::nodeSelectAll() {
    nodeSelectNone();
    elementTree.rootNode.selected = true;
    selectedNodes.insert(&elementTree.rootNode);
}

void Controller::nodeSelectNone() {
    for (const auto &node : selectedNodes) {
        node->selected = false;
    }
    selectedNodes.clear();
}

void Controller::setStandard3dView(int i) {
    renderer.camera.setStandardView(i);
    renderer.unrenderedChanges = true;
}

void window_size_callback(GLFWwindow *window, int width, int height) {
    Controller::getInstance()->setWindowSize(width, height);
}
