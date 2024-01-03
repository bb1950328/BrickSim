#include "scene.h"
#include "../config/read.h"
#include "../controller.h"
#include "../metrics.h"
#include "shaders.h"
#include <palanteer.h>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <spdlog/spdlog.h>

#ifdef BRICKSIM_USE_RENDERDOC

    #include <renderdoc_app.h>

#endif
namespace bricksim::graphics {
    CompleteFramebuffer::CompleteFramebuffer(glm::usvec2 size_) :
        size(size_) {// NOLINT(cppcoreguidelines-pro-type-member-init)
        controller::executeOpenGL([this]() {
            allocate();
        });
    }

    void CompleteFramebuffer::allocate() {
        // create a color attachment texture
        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        glGenRenderbuffers(1, &renderBufferObject);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);// use a single renderbuffer object for both a depth AND stencil buffer.

        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);// now actually attach it

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

    void CompleteFramebuffer::saveImage(const std::filesystem::path& path) const {
        spdlog::info("saveImage(\"{}\")", path.string());
        const int channels = 4;

        auto data = std::vector<GLubyte>();
        data.resize((size_t)size.x * size.y * channels);
        controller::executeOpenGL([this, &data]() {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
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

    const glm::usvec2& CompleteFramebuffer::getSize() const {
        return size;
    }

    void CompleteFramebuffer::setSize(const glm::usvec2& newSize) {
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

    Scene::Scene(scene_id_t sceneId) :
        id(sceneId),
        meshCollection(sceneId),
        backgroundColor(config::get().graphics.background.asGlmVector(), 1.f) {
        setImageSize({10, 10});
    }

    unsigned int Scene::getSelectionPixel(framebuffer_size_t x, framebuffer_size_t y) {
        bool needRender = false;
        if (!selection.has_value()) {
            selection.emplace(imageSize);
            needRender = true;
        } else if (selection->getSize() != imageSize) {
            selection->setSize(imageSize);
            needRender = true;
        }

        if (currentSelectionImageViewMatrix != camera->getViewMatrix()) {
            needRender = true;
            currentSelectionImageViewMatrix = camera->getViewMatrix();
        }
        if (selectionImageEtreeVersion < rootNode->getVersion()) {
            selectionImageEtreeVersion = rootNode->getVersion();
            needRender = true;
        }

        meshCollection.rereadElementTreeIfNeeded();
        std::array<GLubyte, 3> middlePixel{};

        if (!needRender) {
            controller::executeOpenGL([this, &middlePixel, &x, &y]() {
                glUseProgram(0);
                glBindFramebuffer(GL_FRAMEBUFFER, selection->getFBO());
                glReadPixels(x, (selection->getSize().y - y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, middlePixel.data());
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            });
        } else {
            const glm::mat4 projectionView = projectionMatrix * camera->getViewMatrix();
            controller::executeOpenGL([this, &projectionView, &middlePixel, &x, &y]() {
#ifdef BRICKSIM_USE_RENDERDOC
                const auto* renderdocApi = controller::getRenderdocAPI();
                if (renderdocApi) {
                    renderdocApi->StartFrameCapture(nullptr, nullptr);
                }
#endif
                glBindFramebuffer(GL_FRAMEBUFFER, selection->getFBO());
                glViewport(0, 0, imageSize.x, imageSize.y);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                const auto& triangleSelectionShader = shaders::get(shaders::shader_id_t::TRIANGLE_SELECTION);
                const auto& texturedTriangleSelectionShader = shaders::get(shaders::shader_id_t::TEXTURED_TRIANGLE_SELECTION);
                triangleSelectionShader.use();
                triangleSelectionShader.setMat4("projectionView", projectionView);
                texturedTriangleSelectionShader.use();
                texturedTriangleSelectionShader.setMat4("projectionView", projectionView);
                for (const auto& layer: meshCollection.getLayersInUse()) {
                    glClear(GL_DEPTH_BUFFER_BIT);
                    triangleSelectionShader.use();
                    meshCollection.drawTriangleGraphics(layer);
                    texturedTriangleSelectionShader.use();
                    meshCollection.drawTexturedTriangleGraphics(layer);
                }
                glUseProgram(0);

                glReadPixels(x, (selection->getSize().y - y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, middlePixel.data());
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

    const std::shared_ptr<etree::Node>& Scene::getRootNode() const {
        return rootNode;
    }

    void Scene::setRootNode(const std::shared_ptr<etree::Node>& newRootNode) {
        rootNode = newRootNode;
        meshCollection.setRootNode(rootNode);
        imageEtreeVersion = 0;
    }

    const std::shared_ptr<Camera>& Scene::getCamera() const {
        return camera;
    }

    void Scene::setCamera(const std::shared_ptr<Camera>& newCamera) {
        camera = newCamera;
    }

    const glm::usvec2& Scene::getImageSize() const {
        return imageSize;
    }

    void Scene::setImageSize(const glm::usvec2& newImageSize) {
        projectionMatrix = glm::perspective(glm::radians(50.0f), (float)newImageSize.x / (float)newImageSize.y, 0.01f, 1000.0f);
        Scene::imageSize = newImageSize;
    }

    void Scene::updateImage() {
        bool needRender = false;
        if (currentImageViewMatrix != camera->getViewMatrix()) {
            currentImageViewMatrix = camera->getViewMatrix();
            needRender = true;
        }

        meshCollection.rereadElementTreeIfNeeded();

        bool imageSizeChanged = imageSize != image.getSize();
        if (imageSizeChanged) {
            image.setSize(imageSize);
            needRender = true;
        }

        if (overlayCollection.hasChangedElements() || imageSizeChanged) {
            overlayCollection.updateVertices(imageSize);
            needRender = true;
        }

        if (currentImageDrawTriangles != drawTriangles || currentImageDrawLines != drawLines) {
            currentImageDrawTriangles = drawTriangles;
            currentImageDrawLines = drawLines;
            needRender = true;
        }

        if (imageEtreeVersion < rootNode->getVersion()) {
            imageEtreeVersion = rootNode->getVersion();
            needRender = true;
        }

        if (needRender) {
            auto before = std::chrono::high_resolution_clock::now();
            controller::executeOpenGL([this]() {
                plScope("scene render");
#ifdef BRICKSIM_USE_RENDERDOC
                const auto* renderdocApi = controller::getRenderdocAPI();
                if (renderdocApi) {
                    renderdocApi->StartFrameCapture(nullptr, nullptr);
                }
#endif
                glBindFramebuffer(GL_FRAMEBUFFER, image.getFBO());
                glViewport(0, 0, imageSize.x, imageSize.y);
                glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
                glClear(GL_COLOR_BUFFER_BIT);

                glm::mat4 view = camera->getViewMatrix();
                const glm::mat4& projectionView = projectionMatrix * view;

                const auto& triangleShader = shaders::get(shaders::shader_id_t::TRIANGLE);
                const auto& textureShader = shaders::get(shaders::shader_id_t::TEXTURED_TRIANGLE);
                const auto& lineShader = shaders::get(shaders::shader_id_t::LINE);
                const auto& optionalLineShader = shaders::get(shaders::shader_id_t::OPTIONAL_LINE);

                if (drawTriangles) {
                    triangleShader.use();
                    triangleShader.setVec3("viewPos", camera->getCameraPos());
                    triangleShader.setMat4("projectionView", projectionView);
                    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
                    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
                    glm::vec3 ambientColor = diffuseColor * glm::vec3(1.3f);
                    triangleShader.setVec3("light.position", camera->getCameraPos());

                    //todo do not call these every time
                    triangleShader.setVec3("light.ambient", ambientColor);
                    triangleShader.setVec3("light.diffuse", diffuseColor);
                    triangleShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

                    textureShader.use();
                    textureShader.setMat4("projectionView", projectionView);
                }

                for (const auto& layer: meshCollection.getLayersInUse()) {
                    glClear(GL_DEPTH_BUFFER_BIT);
                    if (drawTriangles) {
                        plScope("draw triangles");
                        triangleShader.use();
                        meshCollection.drawTriangleGraphics(layer);

                        textureShader.use();
                        meshCollection.drawTexturedTriangleGraphics(layer);
                    }

                    if (drawLines) {
                        plScope("draw lines");
                        lineShader.use();
                        lineShader.setMat4("projectionView", projectionView);
                        meshCollection.drawLineGraphics(layer);

                        optionalLineShader.use();
                        optionalLineShader.setMat4("projectionView", projectionView);
                        meshCollection.drawOptionalLineGraphics(layer);
                    }
                }

                if (overlayCollection.hasElements()) {
                    plScope("draw overlays");
                    glClear(GL_DEPTH_BUFFER_BIT);
                    shaders::get(shaders::shader_id_t::OVERLAY).use();
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

            auto after = std::chrono::high_resolution_clock::now();
            metrics::lastSceneRenderTimeMs = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(after - before).count()) / 1000.f;
        }
    }

    const CompleteFramebuffer& Scene::getImage() const {
        return image;
    }

    const std::optional<CompleteFramebuffer>& Scene::getSelectionImage() const {
        return selection;
    }

    const mesh::SceneMeshCollection& Scene::getMeshCollection() const {
        return meshCollection;
    }

    overlay2d::ElementCollection& Scene::getOverlayCollection() {
        return overlayCollection;
    }

    glm::vec3 Scene::worldToScreenCoordinates(glm::vec3 worldCoords) const {
        const glm::mat4 projectionView = projectionMatrix * camera->getViewMatrix();
        const auto gl_Position = projectionView * glm::vec4(worldCoords, 1.f);
        const glm::vec3 ndc = gl_Position / gl_Position.w;
        return (ndc + 1.0f) / 2.0f * glm::vec3(imageSize, 1.f);
    }

    unsigned int Scene::getSelectionPixel(glm::usvec2 coords) {
        return getSelectionPixel(coords.x, coords.y);
    }

    Ray3 Scene::screenCoordinatesToWorldRay(glm::usvec2 screenCoords) const {
        //https://stackoverflow.com/a/30005258/8733066
        const auto projectionView = projectionMatrix * camera->getViewMatrix();
        const auto ndc = glm::vec2(screenCoords) / glm::vec2(imageSize) * 2.0f - 1.0f;
        const auto inverseProjectionView = glm::inverse(projectionView);
        auto worldPos = inverseProjectionView * glm::vec4(ndc.x, -ndc.y, 1.0f, 1.0f);
        const auto cameraPos = camera->getCameraPos();
        worldPos /= worldPos.w;
        return {cameraPos, glm::normalize(glm::vec3(worldPos) - cameraPos)};
    }

    [[nodiscard]] bool* Scene::isDrawTriangles() {
        return &drawTriangles;
    }
    [[nodiscard]] bool* Scene::isDrawLines() {
        return &drawLines;
    }
    const glm::vec4& Scene::getBackgroundColor() const {
        return backgroundColor;
    }
    void Scene::setBackgroundColor(const color::RGB& rgbColor) {
        Scene::backgroundColor = {rgbColor.asGlmVector(), 1.f};
    }
    void Scene::setBackgroundColor(const glm::vec4& newColor) {
        Scene::backgroundColor = newColor;
    }

    namespace scenes {
        namespace {
            uomap_t<scene_id_t, std::shared_ptr<Scene>> createdScenes;
        }

        std::shared_ptr<Scene> create(scene_id_t sceneId) {
            auto it = createdScenes.find(sceneId);
            if (it != createdScenes.end()) {
                throw std::invalid_argument("scene with this id already created");
            }
            return createdScenes[sceneId] = std::make_shared<Scene>(sceneId);
        }

        std::shared_ptr<Scene>& get(scene_id_t sceneId) {
            auto it = createdScenes.find(sceneId);
            if (it == createdScenes.end()) {
                throw std::invalid_argument("scene with this id not created yet");
            }
            return it->second;
        }

        void deleteAll() {
            createdScenes.clear();
        }

        uomap_t<scene_id_t, std::shared_ptr<Scene>>& getAll() {
            return createdScenes;
        }

        void remove(scene_id_t sceneId) {
            createdScenes.erase(sceneId);
        }
    }
}
