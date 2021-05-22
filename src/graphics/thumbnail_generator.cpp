#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include "thumbnail_generator.h"
#include "../metrics.h"
#include "../controller.h"
#include "../config.h"

unsigned int ThumbnailGenerator::getThumbnail(const std::shared_ptr<LdrFile> &ldrFile, const LdrColorReference color) {
    if (renderedRotationDegrees != rotationDegrees) {
        discardAllImages();
        renderedRotationDegrees = rotationDegrees;
    }
    file_key_t fileKey = {ldrFile, color};
    auto imgIt = images.find(fileKey);
    if (imgIt == images.end()) {
        spdlog::debug("rendering thumbnail {} in {}", ldrFile->getDescription(), color.get()->name);
        auto before = std::chrono::high_resolution_clock::now();
        scene->setImageSize({size, size});

        auto partNode = std::make_shared<etree::PartNode>(ldrFile, color, nullptr);
        scene->setRootNode(partNode);
        camera->setRootNode(partNode);

        scene->updateImage();

        const auto totalBufferSize = size * size * 3;
        metrics::thumbnailBufferUsageBytes += totalBufferSize;
        auto buffer = std::make_unique<GLbyte[]>(totalBufferSize);
        controller::executeOpenGL([&](){
            //todo copy image directrly (VRAM -> VRAM instead of VRAM -> RAM -> VRAM)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, scene->getImage().getFBO());
            glReadPixels(0, 0, size, size, GL_RGB, GL_UNSIGNED_BYTE, buffer.get());
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            unsigned int textureId;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.get());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            //glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, size, size, 0);
            //t odo this causes source=API, type=ERROR, id=1282: Error has been generated. GL error GL_INVALID_OPERATION in FramebufferTexture2D: (ID: 2333930068) Generic error
            //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
            images[fileKey] = textureId;
        });
        auto after = std::chrono::high_resolution_clock::now();
        metrics::lastThumbnailRenderingTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0;
    }
    lastAccessed.remove(fileKey);
    lastAccessed.push_back(fileKey);
    return images[fileKey];
}

void ThumbnailGenerator::discardAllImages() {
    controller::executeOpenGL([this](){
        for (const auto &item : images) {
            glDeleteTextures(1, &item.second);
        }
    });

    images.clear();
    lastAccessed.clear();
}

void ThumbnailGenerator::discardOldestImages(int reserve_space_for) {
    controller::executeOpenGL([this, &reserve_space_for](){
        int deletedCount = 0;
        while (lastAccessed.size() > maxCachedThumbnails - reserve_space_for) {
            auto lastAccessedIt = lastAccessed.front();
            glDeleteTextures(1, &images[lastAccessedIt]);
            lastAccessed.remove(lastAccessedIt);
            images.erase(lastAccessedIt);
            deletedCount++;
        }
        metrics::thumbnailBufferUsageBytes -= (size_t)size * size * 3 * deletedCount;
    });
}

std::optional<unsigned int> ThumbnailGenerator::getThumbnailNonBlocking(const std::shared_ptr<LdrFile> &ldrFile, LdrColorReference color) {
    if (renderedRotationDegrees != rotationDegrees) {
        discardAllImages();
        renderedRotationDegrees = rotationDegrees;
    }
    file_key_t fileKey = {ldrFile, color};
    auto imgIt = images.find(fileKey);
    if (imgIt == images.end()) {
        if (std::find(renderRequests.begin(), renderRequests.end(), fileKey) == renderRequests.end()) {
            renderRequests.push_back(fileKey);
        }
        return {};
    } else {
        return imgIt->second;
    }
}

bool ThumbnailGenerator::workOnRenderQueue() {
    if (!renderRequests.empty()) {
        const auto &request = renderRequests.front();
        getThumbnail(request.first, request.second);
        renderRequests.pop_front();
    }
    return !renderRequests.empty();
}

unsigned int ThumbnailGenerator::copyImageToTexture() const {
    //todo this is from https://stackoverflow.com/questions/15306899/is-it-possible-to-copy-data-from-one-framebuffer-to-another-in-opengl but it didn't work
    unsigned int destinationTextureId;
    controller::executeOpenGL([this, &destinationTextureId](){
        glGenTextures(1, &destinationTextureId);

        // bind fbo as read / draw fbo
        const auto &image = scene->getImage();
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

bool ThumbnailGenerator::renderQueueEmpty() {
    return renderRequests.empty();
}

bool ThumbnailGenerator::isThumbnailAvailable(const std::shared_ptr<LdrFile> &ldrFile, LdrColorReference color) {
    file_key_t fileKey = {ldrFile, color};
    return images.find(fileKey) != images.end();
}

void ThumbnailGenerator::removeFromRenderQueue(const std::shared_ptr<LdrFile> &ldrFile, LdrColorReference color) {
    file_key_t fileKey = {ldrFile, color};
    auto it = std::find(renderRequests.begin(), renderRequests.end(), fileKey);
    if (it != renderRequests.end()) {
        renderRequests.erase(it);
    }
}

ThumbnailGenerator::ThumbnailGenerator()
        : camera(std::make_shared<FitContentCamera>()),
          size(config::get(config::THUMBNAIL_SIZE)),
          projection(glm::perspective(glm::radians(50.0f), 1.0f, 0.001f, 1000.0f)),
          rotationDegrees(glm::vec3(45, -45, 0)) {
    maxCachedThumbnails = config::get(config::THUMBNAIL_CACHE_SIZE_BYTES) / 3 / size / size;
    scene = scenes::create(scenes::THUMBNAIL_SCENE_ID);
    scene->setCamera(camera);
}
