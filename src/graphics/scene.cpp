#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <glm/ext/matrix_clip_space.hpp>
#include "scene.h"
#include "../controller.h"
#include "../helpers/util.h"
#include "shaders.h"
#include "../config.h"
#include "../metrics.h"

#ifdef BRICKSIM_USE_RENDERDOC

#include <renderdoc.h>

#endif
namespace bricksim::graphics {
    CompleteFramebuffer::CompleteFramebuffer(glm::usvec2 size_) : size(size_) { // NOLINT(cppcoreguidelines-pro-type-member-init)
        controller::executeOpenGL([this]() {
            allocate();
        });
    }

    void CompleteFramebuffer::allocate() {
        // create a color attachment texture
        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        glGenRenderbuffers(1, &renderBufferObject);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y); // use a single renderbuffer object for both a depth AND stencil buffer.

        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject); // now actually attach it


        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            spdlog::error("Framebuffer is not complete");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        spdlog::debug("allocated CompleteFramebuffer. FBO={}, TexBO={}, RBO={}", framebuffer, textureColorbuffer, renderBufferObject);
    }

    CompleteFramebuffer::~CompleteFramebuffer() {
        controller::executeOpenGL([this]() {
            deallocate();
        });
    }

    void CompleteFramebuffer::deallocate() const {
        glDeleteRenderbuffers(1, &renderBufferObject);
        glDeleteTextures(1, &textureColorbuffer);
        glDeleteFramebuffers(1, &framebuffer);
        spdlog::debug("deleted CompleteFramebuffer. FBO={}, TexBO={}, RBO={}", framebuffer, textureColorbuffer, renderBufferObject);
    }

    void CompleteFramebuffer::saveImage(const std::filesystem::path &path) const {
        spdlog::info("saveImage(\"{}\")", path.string());
        const int channels = 3;

        auto data = std::vector<GLubyte>();
        data.resize((size_t) size.x * size.y * channels);
        controller::executeOpenGL([this, &data]() {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        });

        const bool success = util::writeImage(path.string().c_str(), &data[0], size.x, size.y, channels);
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
        controller::executeOpenGL([this]() {
            deallocate();
            allocate();
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
        if (!selection.has_value()) {
            selection.emplace(imageSize);
            selectionImageUpToDate = false;
        } else if (selection->getSize() != imageSize) {
            selection->setSize(imageSize);
            selectionImageUpToDate = false;
        }

        if (currentSelectionImageViewMatrix != camera->getViewMatrix()) {
            selectionImageUpToDate = false;
            currentSelectionImageViewMatrix = camera->getViewMatrix();
        }

        rereadElementTreeIfNeeded();
        GLubyte middlePixel[3];

        if (selectionImageUpToDate) {
            controller::executeOpenGL([&]() {
                glUseProgram(0);
                glBindFramebuffer(GL_FRAMEBUFFER, selection->getFBO());
                glReadPixels(x, (selection->getSize().y - y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, middlePixel);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            });
        } else {
            const glm::mat4 projectionView = projectionMatrix * camera->getViewMatrix();
            controller::executeOpenGL([&]() {
#ifdef BRICKSIM_USE_RENDERDOC
                const auto *renderdocApi = controller::getRenderdocAPI();
                if (renderdocApi) {
                    renderdocApi->StartFrameCapture(nullptr, nullptr);
                }
#endif
                glBindFramebuffer(GL_FRAMEBUFFER, selection->getFBO());
                glViewport(0, 0, imageSize.x, imageSize.y);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                const auto &triangleSelectionShader = shaders::get(shaders::TRIANGLE_SELECTION);
                const auto &texturedTriangleSelectionShader = shaders::get(shaders::TEXTURED_TRIANGLE_SELECTION);
                triangleSelectionShader.use();
                triangleSelectionShader.setMat4("projectionView", projectionView);
                texturedTriangleSelectionShader.use();
                texturedTriangleSelectionShader.setMat4("projectionView", projectionView);
                for (const auto &layer : meshCollection.getLayersInUse()) {
                    glClear(GL_DEPTH_BUFFER_BIT);
                    triangleSelectionShader.use();
                    meshCollection.drawTriangleGraphics(layer);
                    texturedTriangleSelectionShader.use();
                    meshCollection.drawTexturedTriangleGraphics(layer);
                }
                glUseProgram(0);
                selectionImageUpToDate = true;

                glReadPixels(x, (selection->getSize().y - y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, middlePixel);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifdef BRICKSIM_USE_RENDERDOC
                if (renderdocApi) {
                    renderdocApi->EndFrameCapture(nullptr, nullptr);
                }
#endif
            });
        }
        return color::getIntFromColor(middlePixel[0], middlePixel[1], middlePixel[2]);
    }

    void Scene::rereadElementTreeIfNeeded() {
        if (elementTreeRereadNeeded) {
            meshCollection.rereadElementTree();
            elementTreeRereadNeeded = false;
            imageUpToDate = false;
            selectionImageUpToDate = false;
        }
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
        projectionMatrix = glm::perspective(glm::radians(50.0f), (float) newImageSize.x / (float) newImageSize.y, 0.01f, 1000.0f);
        Scene::imageSize = newImageSize;
    }

    void Scene::updateImage() {
        if (currentImageViewMatrix != camera->getViewMatrix()) {
            imageUpToDate = false;
            currentImageViewMatrix = camera->getViewMatrix();
        }

        rereadElementTreeIfNeeded();

        bool imageSizeChanged = imageSize != image.getSize();
        if (imageSizeChanged) {
            image.setSize(imageSize);
            imageUpToDate = false;
        }

        if (overlayCollection.hasChangedElements() || imageSizeChanged) {
            overlayCollection.updateVertices(imageSize);
            imageUpToDate = false;
        }

        if (!imageUpToDate) {
            auto before = std::chrono::high_resolution_clock::now();
            controller::executeOpenGL([this]() {
#ifdef BRICKSIM_USE_RENDERDOC
                const auto *renderdocApi = controller::getRenderdocAPI();
                if (renderdocApi) {
                    renderdocApi->StartFrameCapture(nullptr, nullptr);
                }
#endif
                glBindFramebuffer(GL_FRAMEBUFFER, image.getFBO());
                //spdlog::error("rendering to texture {}", image.getTexBO());
                glViewport(0, 0, imageSize.x, imageSize.y);
                const auto &bgColor = config::get(config::BACKGROUND_COLOR).asGlmVector();
                glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                glm::mat4 view = camera->getViewMatrix();
                const glm::mat4 &projectionView = projectionMatrix * view;

                const auto &triangleShader = shaders::get(shaders::TRIANGLE);
                const auto &textureShader = shaders::get(shaders::TEXTURED_TRIANGLE);
                const auto &lineShader = shaders::get(shaders::LINE);
                const auto &optionalLineShader = shaders::get(shaders::OPTIONAL_LINE);
                triangleShader.use();
                triangleShader.setVec3("viewPos", camera->getCameraPos());
                triangleShader.setMat4("projectionView", projectionView);
                glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
                glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
                glm::vec3 ambientColor = diffuseColor * glm::vec3(1.3f);
                triangleShader.setVec3("light.position", camera->getCameraPos());

                //todo do not call these every time
                triangleShader.setVec3("light.ambient", ambientColor);
                triangleShader.setVec3("light.diffuse", diffuseColor);
                triangleShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

                textureShader.use();
                textureShader.setMat4("projectionView", projectionView);

                for (const auto &layer : meshCollection.getLayersInUse()) {
                    glClear(GL_DEPTH_BUFFER_BIT);
                    triangleShader.use();
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

                if (overlayCollection.hasElements()) {
                    glClear(GL_DEPTH_BUFFER_BIT);
                    shaders::get(shaders::OVERLAY).use();
                    overlayCollection.draw();
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glUseProgram(0);
                glFinish();
#ifdef BRICKSIM_USE_RENDERDOC
                if (renderdocApi) {
                    renderdocApi->EndFrameCapture(nullptr, nullptr);
                }
#endif
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

    const std::optional<CompleteFramebuffer> &Scene::getSelectionImage() const {
        return selection;
    }

    const mesh::SceneMeshCollection &Scene::getMeshCollection() const {
        return meshCollection;
    }

    overlay2d::ElementCollection &Scene::getOverlayCollection() {
        return overlayCollection;
    }

    glm::usvec2 Scene::worldToScreenCoordinates(glm::vec3 worldCoords) {
        const auto projectionView = projectionMatrix * camera->getViewMatrix();
        const auto gl_Position = projectionView * glm::vec4(worldCoords, 1.0f);
        const glm::vec2 ndc = {gl_Position.x / gl_Position.w,
                               gl_Position.y / gl_Position.w};
        return (ndc + 1.0f) / 2.0f * glm::vec2(imageSize);
    }

    unsigned int Scene::getSelectionPixel(glm::usvec2 coords) {
        return getSelectionPixel(coords.x, coords.y);
    }

    Ray3 Scene::screenCoordinatesToWorldRay(glm::usvec2 screenCoords) {
        //https://stackoverflow.com/a/30005258/8733066
        const auto projectionView = projectionMatrix * camera->getViewMatrix();
        const auto ndc = glm::vec2(screenCoords) / glm::vec2(imageSize) * 2.0f - 1.0f;
        const auto inverseProjectionView = glm::inverse(projectionView);
        auto worldPos = inverseProjectionView * glm::vec4(ndc.x, -ndc.y, 1.0f, 1.0f);
        const auto cameraPos = camera->getCameraPos();
        worldPos /= worldPos.w;
        return {cameraPos, glm::normalize(glm::vec3(worldPos) - cameraPos)};
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

        std::map<scene_id_t, std::shared_ptr<Scene>> &getAll() {
            return createdScenes;
        }
    }
}