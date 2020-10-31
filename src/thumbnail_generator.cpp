// thumbnail_generator.cpp
// Created by bab21 on 23.10.20.
//

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "thumbnail_generator.h"
#include "config.h"
#include "ldr_colors.h"

unsigned int ThumbnailGenerator::getThumbnail(const LdrFile* ldrFile) {
    if (renderedRotationDegrees!=rotationDegrees) {
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

        } else {
            mesh = new Mesh();
            meshCollection->meshes[meshKey] = mesh;
            mesh->name = ldrFile->getDescription();
            //todo make color customizable
            mesh->addLdrFile(*ldrFile, glm::mat4(1.0f), LdrColorRepository::getInstance()->get_color(1), false);
        }
        const auto &minimalEnclosingBall = mesh->getMinimalEnclosingBall();
        glm::vec3 center = glm::vec4(minimalEnclosingBall.first, 1.0f)*mesh->globalModel;
        //std::cout << glm::to_string(minPos) << " ... " << glm::to_string(center) << " ... " << glm::to_string(maxPos) << std::endl;
        auto meshDiameter = minimalEnclosingBall.second*constants::LDU_TO_OPENGL;
        //std::cout << "meshDiameter: " << meshDiameter << std::endl;

        auto viewPos = glm::vec3(meshDiameter*3, 0, 0);//todo make mesh centered on framebuffer
        auto view = glm::lookAt(viewPos,
                                glm::vec3(0, 0, 0),//centering is achieved through the instance matrix
                                glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 &projectionView = projection * view;
        auto transformation = glm::eulerAngleYZX(
                (float)(rotationDegrees.x/180*M_PI),
                (float)(rotationDegrees.y/180*M_PI),
                (float)(rotationDegrees.z/180*M_PI)
                );
        transformation = glm::mat4(1.0f);
        transformation = glm::translate(transformation, -center);
        MeshInstance tmpInstance{
                LdrColorRepository::getInstance()->get_color(1),
                transformation,
                0
        };
        mesh->instances.push_back(tmpInstance);
        mesh->instancesHaveChanged = true;
        mesh->writeGraphicsData();
        glViewport(0, 0, size, size);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST); // todo check if this is needed
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer->triangleShader->use();
        renderer->triangleShader->setVec3("viewPos", viewPos);
        renderer->triangleShader->setMat4("projectionView", projectionView);
        renderer->triangleShader->setInt("drawSelection", 0);
        mesh->drawTriangleGraphics();
        //renderer->lineShader->use();
        //renderer->lineShader->setMat4("projectionView", projectionView);
        //mesh->drawLineGraphics();//todo make this work

        const auto totalBufferSize = size * size * 3;
        statistic::Counters::thumbnailBufferUsageBytes += totalBufferSize;
        //GLbyte buffer[totalBufferSize];
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

        glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);
    }
    lastAccessed.remove(ldrFile);
    lastAccessed.push_back(ldrFile);
    return images[ldrFile];
}

void ThumbnailGenerator::discardAllImages() {
    for (const auto &item : images) {
        glDeleteTextures(1, &item.second);
    }
    images.clear();
    lastAccessed.clear();
}

unsigned int ThumbnailGenerator::copyFramebufferToTexture() const {
    //todo this is from https://stackoverflow.com/questions/15306899/is-it-possible-to-copy-data-from-one-framebuffer-to-another-in-opengl but it didn't work
    unsigned int destinationTextureId;
    glGenTextures(1, &destinationTextureId);

    // bind fbo as read / draw fbo
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,framebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    // bind source texture to color attachment
    glBindTexture(GL_TEXTURE_2D,textureBuffer);
    glFramebufferTexture2D(GL_TEXTURE_2D, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureBuffer, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // bind destination texture to another color attachment
    glBindTexture(GL_TEXTURE_2D,destinationTextureId);
    glFramebufferTexture2D(GL_TEXTURE_2D, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, destinationTextureId, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT1);


    // specify source, destination drawing (sub)rectangles.
    glBlitFramebuffer(0, 0, size, size,
                      0, 0, size, size, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // release state
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    return destinationTextureId;
}

ThumbnailGenerator::ThumbnailGenerator(Renderer *renderer) : renderer(renderer), meshCollection(&renderer->meshCollection) {
    size = config::get_long(config::THUMBNAIL_SIZE);
    maxCachedThumbnails = config::get_long(config::THUMBNAIL_CACHE_SIZE_BYTES) / 3 / size / size;
    projection = glm::perspective(glm::radians(50.0f), 1.0f, 0.001f, 1000.0f);
    rotationDegrees = glm::vec3(45, -45, 0);
}

void ThumbnailGenerator::discardOldestImages(int reserve_space_for) {
    int deletedCount = 0;
    while (lastAccessed.size()>maxCachedThumbnails-reserve_space_for) {
        auto lastAccessedIt = lastAccessed.front();
        glDeleteTextures(1, &images[lastAccessedIt]);
        lastAccessed.remove(lastAccessedIt);
        images.erase(lastAccessedIt);
        deletedCount++;
    }
    statistic::Counters::thumbnailBufferUsageBytes -= size * size * 3 * deletedCount;
}

void ThumbnailGenerator::cleanup() {
    Renderer::deleteFramebuffer(&framebuffer, &textureBuffer, &renderBuffer);
}

std::optional<unsigned int> ThumbnailGenerator::getThumbnailNonBlocking(const LdrFile *ldrFile) {
    if (renderedRotationDegrees!=rotationDegrees) {
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

void ThumbnailGenerator::workOnRenderQueue() {
    if (!renderRequests.empty()) {
        getThumbnail(renderRequests.front());
        renderRequests.pop();
    }
}
