//
// Created by bb1950328 on 07.10.2020.
//

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <utility>
#include "renderer.h"
#include "controller.h"
#include "ldr_files/ldr_colors.h"
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

bool Renderer::initialize() {
    if (setupCalled) {
        return true;
    }
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());

    triangleShader = new Shader("src/shaders/triangle_shader.vsh", "src/shaders/triangle_shader.fsh");
    lineShader = new Shader("src/shaders/line_shader.vsh", "src/shaders/line_shader.fsh");
    optionalLineShader = new Shader("src/shaders/optional_line_shader.vsh", "src/shaders/line_shader.fsh", "src/shaders/optional_line_shader.gsh");
    textureShader = new Shader("src/shaders/texture_shader.vsh", "src/shaders/texture_shader.fsh");

    meshCollection->rereadElementTree();
    meshCollection->initializeGraphics();

    updateProjectionMatrix();

    triangleShader->use();
    triangleShader->setVec3("light.position", /*lightPos*/camera.getCameraPos()+glm::vec3(0, 5, 0));
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
    glm::vec3 ambientColor = diffuseColor * glm::vec3(1.3f);
    triangleShader->setVec3("light.ambient", ambientColor);
    triangleShader->setVec3("light.diffuse", diffuseColor);
    triangleShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    createFramebuffer(&imageFramebuffer, &imageTextureColorbuffer, &imageRenderBufferObject);

    setupCalled = true;
    return true;
}

void Renderer::createFramebuffer(unsigned int* framebufferIdLocation,
                                 unsigned int* textureColorbufferIdLocation,
                                 unsigned int* renderBufferObjectIdLocation) {
    createFramebuffer(framebufferIdLocation, textureColorbufferIdLocation, renderBufferObjectIdLocation, windowWidth, windowHeight);
}

void Renderer::createFramebuffer(unsigned int* framebufferIdLocation,
                                 unsigned int* textureColorbufferIdLocation,
                                 unsigned int* renderBufferObjectIdLocation,
                                 unsigned int width,
                                 unsigned int height) {
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
    glGenFramebuffers(1, framebufferIdLocation);
    glBindFramebuffer(GL_FRAMEBUFFER, *framebufferIdLocation);
    // create a color attachment texture

    glGenTextures(1, textureColorbufferIdLocation);
    glBindTexture(GL_TEXTURE_2D, *textureColorbufferIdLocation);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *textureColorbufferIdLocation, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

    glGenRenderbuffers(1, renderBufferObjectIdLocation);
    glBindRenderbuffer(GL_RENDERBUFFER, *renderBufferObjectIdLocation);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *renderBufferObjectIdLocation); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("Framebuffer is not complete");
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
        std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
        glBindFramebuffer(GL_FRAMEBUFFER, imageFramebuffer);
        const util::RGBcolor &bgColor = util::RGBcolor(config::getString(config::BACKGROUND_COLOR));
        glClearColor(bgColor.red/255.0f, bgColor.green/255.0f, bgColor.blue/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        const glm::mat4 &projectionView = projection * view;

        for (const auto &layer : meshCollection->getLayersInUse()) {
            glClear(GL_DEPTH_BUFFER_BIT);
            triangleShader->use();
            triangleShader->setVec3("viewPos", camera.getCameraPos());
            triangleShader->setMat4("projectionView", projectionView);
            triangleShader->setInt("drawSelection", 0);
            meshCollection->drawTriangleGraphics(layer);

            lineShader->use();
            lineShader->setMat4("projectionView", projectionView);
            meshCollection->drawLineGraphics(layer);

            optionalLineShader->use();
            optionalLineShader->setMat4("projectionView", projectionView);
            meshCollection->drawOptionalLineGraphics(layer);
        }


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        unrenderedChanges = false;
    }
    return true;
}

bool Renderer::cleanup() {
    delete lineShader;
    delete optionalLineShader;
    delete triangleShader;
    delete textureShader;
    deleteFramebuffer(&imageFramebuffer, &imageTextureColorbuffer, &imageRenderBufferObject);
    deleteFramebuffer(&selectionFramebuffer, &selectionTextureColorbuffer, &selectionRenderBufferObject);
    return true;
}

Renderer::Renderer(std::shared_ptr<MeshCollection> meshCollection) : meshCollection(std::move(meshCollection)) {
    triangleShader = nullptr;
    lineShader = nullptr;
}

void Renderer::setWindowSize(unsigned int width, unsigned int height) {
    if (windowWidth != width || windowHeight!=height) {
        windowWidth = width;
        windowHeight = height;
        std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
        glViewport(0, 0, width, height);
        updateProjectionMatrix();
        deleteFramebuffer(&imageFramebuffer, &imageTextureColorbuffer, &imageRenderBufferObject);
        createFramebuffer(&imageFramebuffer, &imageTextureColorbuffer, &imageRenderBufferObject);
        unrenderedChanges = true;
    }
}

void Renderer::elementTreeChanged() {
    meshCollection->rereadElementTree();
    unrenderedChanges = true;
}

unsigned int Renderer::getSelectionPixel(unsigned int x, unsigned int y) {
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
    if (currentSelectionBuffersWidth != windowWidth || currentSelectionBuffersHeight != windowHeight) {
        if (currentSelectionBuffersWidth != 0 || currentSelectionBuffersHeight != 0) {
            deleteFramebuffer(&selectionFramebuffer, &selectionTextureColorbuffer, &selectionRenderBufferObject);
        }
        createFramebuffer(&selectionFramebuffer, &selectionTextureColorbuffer, &selectionRenderBufferObject);
        currentSelectionBuffersWidth = windowWidth;
        currentSelectionBuffersHeight = windowHeight;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, selectionFramebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.getViewMatrix();
    const glm::mat4 &projectionView = projection * view;

    triangleShader->use();
    triangleShader->setInt("drawSelection", 1);
    for (const auto &layer : meshCollection->getLayersInUse()) {
        glClear(GL_DEPTH_BUFFER_BIT);
        meshCollection->drawTriangleGraphics(layer);
    }

    GLubyte  middlePixel[3];
    glReadPixels(x, (currentSelectionBuffersHeight - y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, middlePixel);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return util::getIntFromColor(middlePixel[0], middlePixel[1], middlePixel[2]);
}

void Renderer::deleteFramebuffer(unsigned int *framebufferIdLocation,
                                 unsigned int *textureColorbufferIdLocation,
                                 unsigned int *renderBufferObjectIdLocation) {
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
    glDeleteRenderbuffers(1, renderBufferObjectIdLocation);
    glDeleteTextures(1, textureColorbufferIdLocation);
    glDeleteFramebuffers(1, framebufferIdLocation);
    *framebufferIdLocation = 0;
    *textureColorbufferIdLocation = 0;
    *renderBufferObjectIdLocation = 0;
}

bool Renderer::saveImage(const std::string& path) const {
    spdlog::info("saveImage(\"{}\")", path);
    const int channels = 3;

    auto pixels = new GLubyte[windowWidth*windowHeight*channels];
    {
        std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
        glBindFramebuffer(GL_FRAMEBUFFER, imageFramebuffer);
        glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    const bool success = util::writeImage(path.c_str(), pixels, windowWidth, windowHeight, channels);
    delete [] pixels;
    return success;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    gui::setLastScrollDeltaY(yoffset);
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    controller::getRenderer()->camera.moveForwardBackward(yoffset);
    controller::getRenderer()->unrenderedChanges = true;
}
