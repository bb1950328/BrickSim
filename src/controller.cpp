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
    ldr_color_repo::initialize();

    //openFile("test_files/mpd_test.mpd");
    openFile("~/Downloads/arocs.mpd");
    //openFile("3001.dat");

    while (!(glfwWindowShouldClose(window) || userWantsToExit)) {
        const auto loopStart = glfwGetTime();
        auto before = std::chrono::high_resolution_clock::now();
        if (elementTreeChanged) {
            renderer.elementTreeChanged();
            elementTreeChanged = false;
        }
        renderer.loop();
        gui.loop();
        thumbnailGenerator.discardOldestImages(0);
        bool moreWork = true;
        while (glfwGetTime() - loopStart < 1.0 / 60 && moreWork) {
            moreWork = thumbnailGenerator.workOnRenderQueue();
        }
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
    insertLdrElement(ldr_file_repo::get_file(path));
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

void Controller::insertLdrElement(LdrFile *ldrFile) {
    auto currentlyEditingLdrNode = dynamic_cast<etree::LdrNode *>(currentlyEditingNode);
    switch (ldrFile->metaInfo.type) {
        case MODEL:
            currentlyEditingNode = new etree::MpdNode(ldrFile, ldr_color_repo::get_color(2), &elementTree.rootNode);
            elementTree.rootNode.addChild(currentlyEditingNode);
            break;
        case MPD_SUBFILE:
            if (nullptr!=currentlyEditingLdrNode) {
                currentlyEditingLdrNode->addSubfileInstanceNode(ldrFile, ldr_color_repo::get_color(1));
            }
            break;
        case PART:
            if (nullptr!=currentlyEditingLdrNode) {
                currentlyEditingLdrNode->addChild(new etree::PartNode(ldrFile, ldr_color_repo::get_color(1), currentlyEditingNode));
            }
            break;
        default: return;
    }
    elementTreeChanged = true;
}

void Controller::deleteElement(etree::Node *nodeToDelete) {
    nodeToDelete->parent->deleteChild(nodeToDelete);
    selectedNodes.erase(nodeToDelete);
    elementTreeChanged = true;
}

void window_size_callback(GLFWwindow *window, int width, int height) {
    Controller::getInstance()->setWindowSize(width, height);
}
