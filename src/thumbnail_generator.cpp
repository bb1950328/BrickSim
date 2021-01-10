// thumbnail_generator.cpp
// Created by bab21 on 23.10.20.
//

#define GLM_ENABLE_EXPERIMENTAL

#include "lib/stb_image_write.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <mutex>
#include "thumbnail_generator.h"
#include "config.h"
#include "ldr_colors.h"
#include "controller.h"

unsigned int ThumbnailGenerator::getThumbnail(const LdrFile *ldrFile, const LdrColor *color) {
    if (renderedRotationDegrees != rotationDegrees) {
        discardAllImages();
        renderedRotationDegrees = rotationDegrees;
    }
    std::pair<const LdrFile*, const LdrColor*> fileKey = {ldrFile, color}; 
    auto imgIt = images.find(fileKey);
    if (imgIt == images.end()) {
        auto before = std::chrono::high_resolution_clock::now();
        if (framebufferSize != size) {
            renderer->createFramebuffer(&framebuffer, &textureBuffer, &renderBuffer, size, size);
        }
        auto meshKey = std::make_pair((void *) ldrFile, false);
        auto it = meshCollection->meshes.find(meshKey);
        Mesh *mesh;
        std::vector<MeshInstance> instanceBackup;
        if (it != meshCollection->meshes.end()) {
            mesh = it->second;
            instanceBackup = mesh->instances;
            mesh->instances.clear();
        } else {
            mesh = new Mesh();
            meshCollection->meshes[meshKey] = mesh;
            mesh->name = ldrFile->getDescription();
            mesh->addLdrFile(*ldrFile, glm::mat4(1.0f), &ldr_color_repo::getInstanceDummyColor(), false);
        }

        const auto &minimalEnclosingBall = mesh->getMinimalEnclosingBall();
        glm::vec3 center = glm::vec4(minimalEnclosingBall.first, 1.0f) * mesh->globalModel;
        auto meshRadius = minimalEnclosingBall.second * constants::LDU_TO_OPENGL_SCALE;

        MeshInstance tmpInstance{
                color,
                glm::mat4(1.0f),
                0,
                false,
                constants::DEFAULT_LAYER
        };
        mesh->instances.push_back(tmpInstance);
        mesh->instancesHaveChanged = true;
        mesh->writeGraphicsData();

        std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
        auto distance = meshRadius*2.45f;
        auto s = glm::radians(45.0f);//todo make variable
        auto t = glm::radians(45.0f);
        glm::vec3 viewPos = glm::vec3(
                distance * std::cos(s) * std::cos(t),
                distance * std::sin(s) * std::cos(t),
                distance * std::sin(t)
        ) + center;

        auto view = glm::lookAt(viewPos,
                                glm::vec3(0) + center,
                                glm::vec3(0.0f, 1.0f, 0.0f));
        auto projectionView = projection * view;

        {
            renderer->triangleShader->use();
            renderer->triangleShader->setInt("drawSelection", 0);
            glViewport(0, 0, size, size);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderer->triangleShader->setVec3("viewPos", viewPos);
            renderer->triangleShader->setMat4("projectionView", projectionView);
            mesh->drawTriangleGraphics();

            renderer->lineShader->use();
            renderer->lineShader->setMat4("projectionView", projectionView);
            mesh->drawLineGraphics();
            renderer->optionalLineShader->use();
            renderer->optionalLineShader->setMat4("projectionView", projectionView);
            mesh->drawOptionalLineGraphics();

            const auto totalBufferSize = size * size * 3;
            statistic::thumbnailBufferUsageBytes += totalBufferSize;
            auto buffer = std::make_unique<GLbyte[]>(totalBufferSize);
            glReadPixels(0, 0, size, size, GL_RGB, GL_UNSIGNED_BYTE, buffer.get());
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            unsigned int textureId;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.get());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
            images[fileKey] = textureId;
            mesh->instances = instanceBackup;
            mesh->instancesHaveChanged = true;
            mesh->writeGraphicsData();

            glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);
        }
        auto after = std::chrono::high_resolution_clock::now();
        statistic::lastThumbnailRenderingTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0;
    }
    lastAccessed.remove(fileKey);
    lastAccessed.push_back(fileKey);
    return images[fileKey];
}

void ThumbnailGenerator::saveFramebufferToBMP(const std::string &filename) const {
    std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
    auto *pixels = new GLubyte[size * size * 4];
    glReadPixels(0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_write_bmp(filename.c_str(), size, size, 4, pixels);
    delete[] pixels;
}

void ThumbnailGenerator::discardAllImages() {
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
    for (const auto &item : images) {
        glDeleteTextures(1, &item.second);
    }
    images.clear();
    lastAccessed.clear();
}

ThumbnailGenerator::ThumbnailGenerator(Renderer *renderer) : renderer(renderer), meshCollection(&renderer->meshCollection) {
}

void ThumbnailGenerator::initialize() {
    size = config::getInt(config::THUMBNAIL_SIZE);
    maxCachedThumbnails = config::getInt(config::THUMBNAIL_CACHE_SIZE_BYTES) / 3 / size / size;
    projection = glm::perspective(glm::radians(50.0f), 1.0f, 0.001f, 1000.0f);
    rotationDegrees = glm::vec3(45, -45, 0);
}

void ThumbnailGenerator::discardOldestImages(int reserve_space_for) {
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
    int deletedCount = 0;
    while (lastAccessed.size() > maxCachedThumbnails - reserve_space_for) {
        auto lastAccessedIt = lastAccessed.front();
        glDeleteTextures(1, &images[lastAccessedIt]);
        lastAccessed.remove(lastAccessedIt);
        images.erase(lastAccessedIt);
        deletedCount++;
    }
    statistic::thumbnailBufferUsageBytes -= size * size * 3 * deletedCount;
}

void ThumbnailGenerator::cleanup() {
    Renderer::deleteFramebuffer(&framebuffer, &textureBuffer, &renderBuffer);
}

std::optional<unsigned int> ThumbnailGenerator::getThumbnailNonBlocking(const LdrFile *ldrFile, const LdrColor *color) {
    if (renderedRotationDegrees != rotationDegrees) {
        discardAllImages();
        renderedRotationDegrees = rotationDegrees;
    }
    std::pair<const LdrFile*, const LdrColor*> fileKey = {ldrFile, color};
    auto imgIt = images.find(fileKey);
    if (imgIt == images.end()) {
        if (std::find(renderRequests.begin(), renderRequests.end(), fileKey)==renderRequests.end()) {
            renderRequests.push_back(fileKey);
        }
        return {};
    } else {
        return imgIt->second;
    }
}

bool ThumbnailGenerator::workOnRenderQueue() {
    if (!renderRequests.empty()) {
        const std::pair<const LdrFile *, const LdrColor *> &request = renderRequests.front();
        getThumbnail(request.first, request.second);
        renderRequests.pop_front();
    }
    return !renderRequests.empty();
}

unsigned int ThumbnailGenerator::copyFramebufferToTexture() const {
    std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
    //todo this is from https://stackoverflow.com/questions/15306899/is-it-possible-to-copy-data-from-one-framebuffer-to-another-in-opengl but it didn't work
    unsigned int destinationTextureId;
    glGenTextures(1, &destinationTextureId);

    // bind fbo as read / draw fbo
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    // bind source texture to color attachment
    glBindTexture(GL_TEXTURE_2D, textureBuffer);
    glFramebufferTexture2D(GL_TEXTURE_2D, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureBuffer, 0);
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
    return destinationTextureId;
}

bool ThumbnailGenerator::renderQueueEmpty() {
    return renderRequests.empty();
}
