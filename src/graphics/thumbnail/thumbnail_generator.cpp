#include "thumbnail_generator.h"
#include "../../config/read.h"
#include "../../controller.h"
#include "../../metrics.h"
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <palanteer.h>
#include <spdlog/spdlog.h>

namespace bricksim::graphics {
    std::shared_ptr<Texture> ThumbnailGenerator::getThumbnail(ThumbnailRequest request) {
        plFunction();
        if (renderedRotationDegrees != rotationDegrees) {
            discardAllImages();
            renderedRotationDegrees = rotationDegrees;
        }
        auto imgIt = images.find(request);
        if (imgIt == images.end()) {
            spdlog::debug("rendering thumbnail {} {} in {}", request.ldrFile->metaInfo.name,
                          request.ldrFile->getDescription(), request.color.get()->name);
            auto before = std::chrono::high_resolution_clock::now();
            scene->setImageSize({size, size});

            std::shared_ptr<etree::RootNode> rootNode = std::make_shared<etree::RootNode>();
            std::shared_ptr<etree::LdrNode> ldrNode;
            if (request.ldrFile->metaInfo.type == ldr::FileType::MODEL || request.ldrFile->metaInfo.type == ldr::FileType::MPD_SUBFILE) {
                ldrNode = std::make_shared<etree::ModelNode>(request.ldrFile, request.color, nullptr);
                ldrNode->visible = true;
            } else {
                ldrNode = std::make_shared<etree::PartNode>(request.ldrFile, request.color, nullptr, nullptr);
            }
            rootNode->addChild(ldrNode);
            ldrNode->parent = rootNode;
            ldrNode->createChildNodes();
            scene->setRootNode(rootNode);
            camera->setRootNode(ldrNode);

            scene->setBackgroundColor(request.backgroundColor.value_or(config::get().graphics.background));
            scene->updateImage();

            const auto totalBufferSize = size * size * 3;//todo try to make transparent background
            metrics::thumbnailBufferUsageBytes += totalBufferSize;
            std::vector<GLbyte> buffer;
            buffer.resize(totalBufferSize);
            controller::executeOpenGL([this, &buffer, &request]() {
                //todo copy image directly (VRAM -> VRAM instead of VRAM -> RAM -> VRAM)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, scene->getImage().getFBO());
                glReadPixels(0, 0, size, size, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                texture_id_t textureId;
                glGenTextures(1, &textureId);
                glBindTexture(GL_TEXTURE_2D, textureId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                //glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, size, size, 0);
                //todo this causes source=API, type=ERROR, id=1282: Error has been generated. GL error GL_INVALID_OPERATION in FramebufferTexture2D: (ID: 2333930068) Generic error
                //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
                images.emplace(request, std::make_shared<Texture>(textureId, size, size, 3));
            });
            auto after = std::chrono::high_resolution_clock::now();
            metrics::lastThumbnailRenderingTimeMs = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(after - before).count()) / 1000.f;
        }
        lastAccessed.remove(request);
        lastAccessed.push_back(request);
        return images[request];
    }

    void ThumbnailGenerator::discardAllImages() {
        images.clear();
        lastAccessed.clear();
    }

    void ThumbnailGenerator::discardOldestImages(size_t reserve_space_for) {
        controller::executeOpenGL([this, &reserve_space_for]() {
            int deletedCount = 0;
            while (lastAccessed.size() > maxCachedThumbnails - reserve_space_for) {
                auto lastAccessedIt = lastAccessed.front();
                lastAccessed.remove(lastAccessedIt);
                images.erase(lastAccessedIt);
                deletedCount++;
            }
            metrics::thumbnailBufferUsageBytes -= (size_t)size * size * 3 * deletedCount;
        });
    }

    std::optional<std::shared_ptr<Texture>> ThumbnailGenerator::getThumbnailNonBlocking(ThumbnailRequest request) {
        if (renderedRotationDegrees != rotationDegrees) {
            discardAllImages();
            renderedRotationDegrees = rotationDegrees;
        }
        auto imgIt = images.find(request);
        if (imgIt == images.end()) {
            if (std::find(renderRequests.begin(), renderRequests.end(), request) == renderRequests.end()) {
                renderRequests.push_back(request);
            }
            return {};
        } else {
            return imgIt->second;
        }
    }

    bool ThumbnailGenerator::workOnRenderQueue() {
        if (!renderRequests.empty()) {
            const auto& request = renderRequests.front();
            getThumbnail(request);
            renderRequests.pop_front();
        }
        return !renderRequests.empty();
    }

    unsigned int ThumbnailGenerator::copyImageToTexture() const {
        //todo this is from https://stackoverflow.com/questions/15306899/is-it-possible-to-copy-data-from-one-framebuffer-to-another-in-opengl but it didn't work
        unsigned int destinationTextureId;
        controller::executeOpenGL([this, &destinationTextureId]() {
            glGenTextures(1, &destinationTextureId);

            // bind fbo as read / draw fbo
            const auto& image = scene->getImage();
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, image.getFBO());
            glBindFramebuffer(GL_READ_FRAMEBUFFER, image.getFBO());

            // bind source texture to color attachment
            glBindTexture(GL_TEXTURE_2D, image.getTexBO());
            glFramebufferTexture2D(GL_TEXTURE_2D, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image.getTexBO(), 0);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            // bind destination texture to another color attachment
            glBindTexture(GL_TEXTURE_2D, destinationTextureId);
            glFramebufferTexture2D(GL_TEXTURE_2D, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, destinationTextureId, 0);
            glReadBuffer(GL_COLOR_ATTACHMENT1);

            // specify source, destination drawing (sub)rectangles.
            glBlitFramebuffer(0, 0, size, size,
                              0, 0, size, size, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            // release state
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        });
        return destinationTextureId;
    }

    bool ThumbnailGenerator::renderQueueEmpty() const {
        return renderRequests.empty();
    }

    bool ThumbnailGenerator::isThumbnailAvailable(const ThumbnailRequest &request) {
        return images.find(request) != images.end();
    }

    std::size_t ThumbnailGenerator::getNumCachedThumbnails() const {
        return images.size();
    }

    void ThumbnailGenerator::removeFromRenderQueue(const ThumbnailRequest &request) {
        auto it = std::find(renderRequests.begin(), renderRequests.end(), request);
        if (it != renderRequests.end()) {
            renderRequests.erase(it);
        }
    }

    ThumbnailGenerator::ThumbnailGenerator() {
        maxCachedThumbnails = 8UL * (1 << 30) / 3 / size / size;
        scene = scenes::create(scenes::THUMBNAIL_SCENE_ID);
        scene->setCamera(camera);
    }

    bool ThumbnailRequest::operator==(const ThumbnailRequest& rhs) const {
        return ldrFile == rhs.ldrFile && color == rhs.color && backgroundColor == rhs.backgroundColor;
    }

    bool ThumbnailRequest::operator!=(const ThumbnailRequest& rhs) const {
        return !(rhs == *this);
    }

    bool ThumbnailRequest::operator<(const ThumbnailRequest& rhs) const {
        if (ldrFile < rhs.ldrFile)
            return true;
        if (rhs.ldrFile < ldrFile)
            return false;
        if (color < rhs.color)
            return true;
        if (rhs.color < color)
            return false;
        return backgroundColor < rhs.backgroundColor;
    }

    bool ThumbnailRequest::operator>(const ThumbnailRequest& rhs) const {
        return rhs < *this;
    }

    bool ThumbnailRequest::operator<=(const ThumbnailRequest& rhs) const {
        return !(rhs < *this);
    }

    bool ThumbnailRequest::operator>=(const ThumbnailRequest& rhs) const {
        return !(*this < rhs);
    }

    std::string ThumbnailRequest::getFilename() {
        return util::escapeFilename(fmt::format("{}_{}_{}.png",
                                                ldrFile->metaInfo.name,
                                                color.code,
                                                backgroundColor.has_value()
                                                    ? backgroundColor->asHtmlCode()
                                                    : ""));
    }
}
