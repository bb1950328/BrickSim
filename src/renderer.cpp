//
// Created by bb1950328 on 07.10.2020.
//

#include <imgui.h>
#include "renderer.h"
#include "controller.h"
#include "ldr_colors.h"

bool Renderer::setup() {
    if (setupCalled) {
        return true;
    }

    triangleShader = new Shader("src/shaders/triangle_shader.vsh", "src/shaders/triangle_shader.fsh");
    lineShader = new Shader("src/shaders/line_shader.vsh", "src/shaders/line_shader.fsh");

    LdrFileRepository::initializeNames();
    auto before = std::chrono::high_resolution_clock::now();

    elementTree->loadLdrFile("~/Downloads/arocs.mpd");

    auto between = std::chrono::high_resolution_clock::now();

    meshCollection.rereadElementTree();

    stats::print();

    updateProjectionMatrix();

    meshCollection.initializeGraphics();

    triangleShader->use();

    triangleShader->setVec3("light.position", lightPos);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.9f); // low influence
    triangleShader->setVec3("light.ambient", ambientColor);
    triangleShader->setVec3("light.diffuse", diffuseColor);
    triangleShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    createFramebuffer();

    setupCalled = true;
    return true;
}

void Renderer::createFramebuffer() {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

    glGenRenderbuffers(1, &renderBufferObject);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject); // now actually attach it
// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::updateProjectionMatrix() {
    projection = glm::perspective(glm::radians(50.0f), (float) windowWidth / (float) windowHeight, 0.1f, 1000.0f);
    unrenderedChanges = true;
}

bool Renderer::loop() {
    if (!setupCalled) {
        throw std::invalid_argument("call setup first!");
    }
    processInput(window);

    if (unrenderedChanges) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST); // todo check if this is needed
        const RGB &bgColor = RGB(config::get_string(config::BACKGROUND_COLOR));
        glClearColor(bgColor.red/255.0f, bgColor.green/255.0f, bgColor.blue/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        const glm::mat4 &projectionView = projection * view;

        triangleShader->use();
        triangleShader->setVec3("viewPos", camera.getCameraPos());
        triangleShader->setMat4("projectionView", projectionView);
        meshCollection.drawTriangleGraphics(triangleShader);
        lineShader->use();
        lineShader->setMat4("projectionView", projectionView);
        meshCollection.drawLineGraphics(lineShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        unrenderedChanges = false;
    }
    return true;
}

bool Renderer::cleanup() {
    meshCollection.deallocateGraphics();
    glfwTerminate();
    return true;
}

Renderer::Renderer(ElementTree *elementTree) : meshCollection(elementTree) {
    this->elementTree = elementTree;
    triangleShader = nullptr;
    lineShader = nullptr;
}

void Renderer::setWindowSize(unsigned int width, unsigned int height) {
    if (windowWidth != width || windowHeight!=height) {
        windowWidth = width;
        windowHeight = height;
        glViewport(0, 0, width, height);
        updateProjectionMatrix();
        createFramebuffer();
        unrenderedChanges = true;
    }
}

void Renderer::elementTreeChanged() {
    meshCollection.rereadElementTree();
    unrenderedChanges = true;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    Controller::getInstance()->gui.lastScrollDeltaY = yoffset;
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    Controller::getInstance()->renderer.camera.moveForwardBackward(yoffset);
    Controller::getInstance()->renderer.unrenderedChanges = true;
}
