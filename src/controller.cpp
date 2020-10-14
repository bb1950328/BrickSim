// controller.cpp
// Created by bab21 on 09.10.20.
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
    renderer.setWindowSize(view3dWidth, view3dHeight);
    renderer.setup();
    gui.setup();
    while (!glfwWindowShouldClose(window)) {
        auto before = std::chrono::high_resolution_clock::now();
        renderer.loop();
        gui.loop();
        auto after = std::chrono::high_resolution_clock::now();
        lastFrameTime = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    config::set_long(config::KEY_SCREEN_WIDTH, windowWidth);
    config::set_long(config::KEY_SCREEN_HEIGHT, windowHeight);
    config::save();
    gui.cleanup();
    renderer.cleanup();
    return 0;
}

Controller *Controller::getInstance() {
    static Controller instance;
    return &instance;
}

Controller::Controller() : renderer(&elementTree) {

}

bool Controller::initializeGL() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, (int)(config::get_long(config::KEY_MSAA_SAMPLES)));
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
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
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

void window_size_callback(GLFWwindow *window, int width, int height) {
    Controller::getInstance()->setWindowSize(width, height);
}
