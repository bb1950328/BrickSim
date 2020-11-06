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

unsigned int ThumbnailGenerator::getThumbnail(const LdrFile *ldrFile) {
    if (renderedRotationDegrees != rotationDegrees) {
        discardAllImages();
        renderedRotationDegrees = rotationDegrees;
    }
    auto imgIt = images.find(ldrFile);
    if (imgIt == images.end()) {
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
            //todo make color customizable
            mesh->addLdrFile(*ldrFile, glm::mat4(1.0f), ldr_color_repo::get_color(1), false);
        }

        const auto &minimalEnclosingBall = mesh->getMinimalEnclosingBall();
        glm::vec3 center = glm::vec4(minimalEnclosingBall.first, 1.0f) * mesh->globalModel;
        auto meshRadius = minimalEnclosingBall.second * constants::LDU_TO_OPENGL;

        MeshInstance tmpInstance{
                ldr_color_repo::get_color(1),
                glm::mat4(1.0f),
                0
        };
        mesh->instances.push_back(tmpInstance);
        mesh->instancesHaveChanged = true;
        mesh->writeGraphicsData();

        int iterations = 0;
        glm::mat4 projectionView;
        CadCamera camera;
        camera.setPitch(45);
        camera.setYaw(45);
        bool allInside;
        float distance = .1;

        renderer->triangleShader->use();
        renderer->triangleShader->setInt("drawSelection", 0);

        {
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            do {
                //distance = (upperLimit + lowerLimit) / 2;
                //std::cout << lowerLimit << " < " << distance << " < " << upperLimit << std::endl;
                camera.setDistance(distance);
                /*glm::vec3 viewPos = glm::vec3(
                        distance * std::cos(s) * std::sin(t),
                        distance * std::sin(s) * std::sin(t),
                        distance * std::cos(t)
                ) - center;*/
                glm::vec3 viewPos = camera.getCameraPos()/*-center*/;
                auto view = glm::lookAt(viewPos,
                                        glm::vec3(0) + center,
                                        glm::vec3(0.0f, 1.0f, 0.0f));
                view = camera.getViewMatrix();
                projectionView = projection * view;
                glViewport(0, 0, size, size);
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                renderer->triangleShader->setVec3("viewPos", viewPos);
                renderer->triangleShader->setMat4("projectionView", projectionView);
                mesh->drawTriangleGraphics();

                allInside = true;
                GLbyte buffer1[3];
                GLbyte buffer2[3];
                GLbyte buffer3[3];
                GLbyte buffer4[3];
                for (int i = 0; i < size; ++i) {//todo this is very inefficent
                    glReadPixels(i, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, buffer1);
                    glReadPixels(0, i, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, buffer2);
                    glReadPixels(i, size - 1, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, buffer3);
                    glReadPixels(size - 1, i, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, buffer4);
                    if (buffer1[0] != 0 || buffer1[1] != 0 || buffer1[2] != 0
                        || buffer2[0] != 0 || buffer2[1] != 0 || buffer2[2] != 0
                        || buffer3[0] != 0 || buffer3[1] != 0 || buffer3[2] != 0
                        || buffer4[0] != 0 || buffer4[1] != 0 || buffer4[2] != 0) {
                        allInside = false;
                        break;
                    }
                }
                iterations++;

                std::string filename =
                        std::string("thumbnail/_") + mesh->name + "_iteration" + std::to_string(iterations) + ".bmp";
                //saveFramebufferToBMP(filename);
                distance += .33;
            } while (/*upperLimit - lowerLimit > 0.001 ||*/ !allInside);
            //std::cout << "iterations: " << iterations << std::endl;

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
            images[ldrFile] = textureId;
            mesh->instances = instanceBackup;
            mesh->instancesHaveChanged = true;
            mesh->writeGraphicsData();

            glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);
        }
    }
    lastAccessed.remove(ldrFile);
    lastAccessed.push_back(ldrFile);
    return images[ldrFile];
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
    size = config::get_long(config::THUMBNAIL_SIZE);
    maxCachedThumbnails = config::get_long(config::THUMBNAIL_CACHE_SIZE_BYTES) / 3 / size / size;
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

std::optional<unsigned int> ThumbnailGenerator::getThumbnailNonBlocking(const LdrFile *ldrFile) {
    if (renderedRotationDegrees != rotationDegrees) {
        discardAllImages();
        renderedRotationDegrees = rotationDegrees;
    }
    auto imgIt = images.find(ldrFile);
    if (imgIt == images.end()) {
        renderRequests.push(ldrFile);
        return {};
    } else {
        return imgIt->second;
    }
}

bool ThumbnailGenerator::workOnRenderQueue() {
    if (!renderRequests.empty()) {
        getThumbnail(renderRequests.front());
        renderRequests.pop();
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
