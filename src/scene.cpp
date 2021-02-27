

#include "scene.h"
#include "controller.h"
#include "latest_log_messages_tank.h"

CompleteFramebuffer::CompleteFramebuffer(framebuffer_size_t width, framebuffer_size_t height) { // NOLINT(cppcoreguidelines-pro-type-member-init)
    currentSize[0] = width;
    currentSize[1] = height;
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // create a color attachment texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &renderBufferObject);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject); // now actually attach it

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("Framebuffer is not complete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

CompleteFramebuffer::~CompleteFramebuffer() {
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
    glDeleteRenderbuffers(1, &renderBufferObject);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteFramebuffers(1, &framebuffer);
}

void CompleteFramebuffer::saveImage(const std::filesystem::path &path) {
    spdlog::info("saveImage(\"{}\")", path.string());
    const int channels = 3;

    auto data = std::vector<GLubyte>();
    data.resize(currentSize[0]*currentSize[1]*channels);
    {
        std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glReadPixels(0, 0, currentSize[0], currentSize[1], GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    const bool success = util::writeImage(path.c_str(), &data[0], currentSize[0], currentSize[1], channels);
    if (!success) {
        throw std::invalid_argument("util::writeImage did not succeed");
    }
}

void CompleteFramebuffer::resize(framebuffer_size_t newWidth, framebuffer_size_t newHeight) {
    if (currentSize[0] == newWidth && currentSize[1] == newHeight) {
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newWidth, newHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, newWidth, newHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    currentSize[0] = newWidth;
    currentSize[1] = newHeight;
}

std::unique_ptr<Shader> Scene::triangleShader = nullptr;
std::unique_ptr<Shader> Scene::lineShader = nullptr;
std::unique_ptr<Shader> Scene::optionalLineShader = nullptr;
std::unique_ptr<Shader> Scene::textureShader = nullptr;
std::unique_ptr<Shader> Scene::overlayShader = nullptr;

Scene::Scene() : image(64, 64) {
    if (textureShader == nullptr) {
        textureShader = std::make_unique<Shader>("src/shaders/triangle_shader.vsh", "src/shaders/triangle_shader.fsh");
        lineShader = std::make_unique<Shader>("src/shaders/line_shader.vsh", "src/shaders/line_shader.fsh");
        optionalLineShader = std::make_unique<Shader>("src/shaders/optional_line_shader.vsh", "src/shaders/line_shader.fsh", "src/shaders/optional_line_shader.gsh");
        textureShader = std::make_unique<Shader>("src/shaders/texture_shader.vsh", "src/shaders/texture_shader.fsh");
        overlayShader = std::make_unique<Shader>("src/shaders/overlay_shader.vsh", "src/shaders/overlay_shader.fsh");
    }
}
