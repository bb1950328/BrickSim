

#include "scene.h"
#include "controller.h"
#include "latest_log_messages_tank.h"
#include "metrics.h"
#include "config.h"

CompleteFramebuffer::CompleteFramebuffer(glm::usvec2 size_): size(size_) { // NOLINT(cppcoreguidelines-pro-type-member-init)
    controller::executeOpenGL([this](){
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // create a color attachment texture
        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        glGenRenderbuffers(1, &renderBufferObject);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject); // now actually attach it

        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            spdlog::error("Framebuffer is not complete");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    });
}

CompleteFramebuffer::~CompleteFramebuffer() {
    controller::executeOpenGL([this](){
        glDeleteRenderbuffers(1, &renderBufferObject);
        glDeleteTextures(1, &textureColorbuffer);
        glDeleteFramebuffers(1, &framebuffer);
    });
}

void CompleteFramebuffer::saveImage(const std::filesystem::path &path) const {
    spdlog::info("saveImage(\"{}\")", path.string());
    const int channels = 3;

    auto data = std::vector<GLubyte>();
    data.resize(size.x * size.y * channels);
    controller::executeOpenGL([this, &data](){
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    });

    const bool success = util::writeImage(path.c_str(), &data[0], size.x, size.y, channels);
    if (!success) {
        throw std::invalid_argument("util::writeImage did not succeed");
    }
}

unsigned int CompleteFramebuffer::getFBO() const {
    return framebuffer;
}

const glm::usvec2 &CompleteFramebuffer::getSize() const {
    return size;
}

void CompleteFramebuffer::setSize(const glm::usvec2 &newSize) {
    if (size == newSize) {
        return;
    }
    size = newSize;
    controller::executeOpenGL([this](){
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    });
}

unsigned int CompleteFramebuffer::getTexBO() const {
    return textureColorbuffer;
}

unsigned int CompleteFramebuffer::getRBO() const {
    return renderBufferObject;
}

Scene::Scene(scene_id_t sceneId) : image({64, 64}), meshCollection(sceneId), id(sceneId), imageUpToDate(false), elementTreeRereadNeeded(false) {
    setImageSize({10, 10});
}

unsigned int Scene::getSelectionPixel(framebuffer_size_t x, framebuffer_size_t y) {
    //todo don't render the selection image again if nothing has changed
    if (selection.has_value()) {
        selection->setSize(imageSize);
    } else {
        selection.emplace(imageSize);
    }
    GLubyte middlePixel[3];
    const glm::mat4 projectionView = projectionMatrix * camera->getViewMatrix();
    controller::executeOpenGL([this, &x, &y, &middlePixel](){
        glBindFramebuffer(GL_FRAMEBUFFER, selection->getFBO());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto& triangleShader = shaders::get(shaders::TRIANGLE);
        const auto& textureShader = shaders::get(shaders::TEXTURED_TRIANGLE);
        triangleShader.use();
        triangleShader.setInt("drawSelection", 1);//todo check if a dedicated selectionTriangleShader is faster
        for (const auto &layer : meshCollection.getLayersInUse()) {
            glClear(GL_DEPTH_BUFFER_BIT);
            triangleShader.use();
            meshCollection.drawTriangleGraphics(layer);
            textureShader.use();
            meshCollection.drawTexturedTriangleGraphics(layer);
        }

        glReadPixels(x, (selection->getSize().y - y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, middlePixel);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    });

    return util::getIntFromColor(middlePixel[0], middlePixel[1], middlePixel[2]);
}

const std::shared_ptr<etree::Node> &Scene::getRootNode() const {
    return rootNode;
}

void Scene::setRootNode(const std::shared_ptr<etree::Node> &newRootNode) {
    rootNode = newRootNode;
    meshCollection.setRootNode(rootNode);
    elementTreeChanged();
}

const std::shared_ptr<Camera> &Scene::getCamera() const {
    return camera;
}

void Scene::setCamera(const std::shared_ptr<Camera> &newCamera) {
    camera = newCamera;
    imageUpToDate = false;
}

const glm::usvec2 &Scene::getImageSize() const {
    return imageSize;
}

void Scene::setImageSize(const glm::usvec2 &newImageSize) {
    projectionMatrix = glm::perspective(glm::radians(50.0f), (float)newImageSize.x / (float)newImageSize.y, 0.1f, 1000.0f);
    Scene::imageSize = newImageSize;
}

void Scene::updateImage() {
    if (lastRenderedViewMatrix != camera->getViewMatrix()) {
        imageUpToDate = false;
        lastRenderedViewMatrix = camera->getViewMatrix();
    }
    if (elementTreeRereadNeeded) {
        meshCollection.rereadElementTree();
        elementTreeRereadNeeded = false;
        imageUpToDate = false;
    }

    if (!imageUpToDate) {
        auto before = std::chrono::high_resolution_clock::now();
        controller::executeOpenGL([this](){
            glBindFramebuffer(GL_FRAMEBUFFER, image.getFBO());
            const auto &bgColor = util::RGBcolor(config::getString(config::BACKGROUND_COLOR)).asGlmVector();
            glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glm::mat4 view = camera->getViewMatrix();
            const glm::mat4 &projectionView = projectionMatrix * view;

            const auto& triangleShader = shaders::get(shaders::TRIANGLE);
            const auto& textureShader = shaders::get(shaders::TEXTURED_TRIANGLE);
            const auto& lineShader = shaders::get(shaders::LINE);
            const auto& optionalLineShader = shaders::get(shaders::OPTIONAL_LINE);
            for (const auto &layer : meshCollection.getLayersInUse()) {
                glClear(GL_DEPTH_BUFFER_BIT);
                triangleShader.use();
                triangleShader.setVec3("viewPos", camera->getCameraPos());
                triangleShader.setMat4("projectionView", projectionView);
                triangleShader.setInt("drawSelection", 0);
                meshCollection.drawTriangleGraphics(layer);

                textureShader.use();
                meshCollection.drawTexturedTriangleGraphics(layer);

                lineShader.use();
                lineShader.setMat4("projectionView", projectionView);
                meshCollection.drawLineGraphics(layer);

                optionalLineShader.use();
                optionalLineShader.setMat4("projectionView", projectionView);
                meshCollection.drawOptionalLineGraphics(layer);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glFinish();
        });

        imageUpToDate = true;

        auto after = std::chrono::high_resolution_clock::now();
        metrics::lastSceneRenderTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0f;
    }
}

void Scene::elementTreeChanged() {
    elementTreeRereadNeeded = true;
}

const CompleteFramebuffer &Scene::getImage() const {
    return image;
}

const std::optional<CompleteFramebuffer>& Scene::getSelectionImage() const {
    return selection;
}

const SceneMeshCollection &Scene::getMeshCollection() const {
    return meshCollection;
}

namespace scenes {
    namespace {
        std::map<scene_id_t, std::shared_ptr<Scene>> createdScenes;
    }
    std::shared_ptr<Scene> create(scene_id_t sceneId) {
        auto it = createdScenes.find(sceneId);
        if (it != createdScenes.end()) {
            throw std::invalid_argument("scene with this id already created");
        }
        return createdScenes[sceneId] = std::make_shared<Scene>(sceneId);
    }

    std::shared_ptr<Scene> get(scene_id_t sceneId) {
        auto it = createdScenes.find(sceneId);
        if (it == createdScenes.end()) {
            throw std::invalid_argument("scene with this id not created yet");
        }
        return it->second;
    }

    void deleteAll() {
        createdScenes.clear();
    }
}