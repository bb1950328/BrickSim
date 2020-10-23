// thumbnail_generator.cpp
// Created by bab21 on 23.10.20.
//

#include "thumbnail_generator.h"
#include "config.h"
#include "ldr_colors.h"

GLbyte *ThumbnailGenerator::getThumbnail(const LdrFile* ldrFile) {
    auto imgIt = images.find(ldrFile);
    if (imgIt != images.end()) {
        auto meshKey = std::make_pair((void *) ldrFile, false);
        auto it = meshCollection->meshes.find(meshKey);
        Mesh *mesh;
        if (it != meshCollection->meshes.end()) {
            mesh = it->second;
        } else {
            mesh = new Mesh();
            meshCollection->meshes[meshKey] = mesh;
            mesh->name = ldrFile->getDescription();
            //todo make color customizable
            mesh->addLdrFile(*ldrFile, glm::mat4(1.0f), LdrColorRepository::getInstance()->get_color(1), false);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST); // todo check if this is needed
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        auto viewPos = glm::vec3(10.0f, 0, 0);
        auto view = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 &projectionView = projection * view;
        renderer->triangleShader->use();
        renderer->triangleShader->setVec3("viewPos", viewPos);
        renderer->triangleShader->setMat4("projectionView", projectionView);
        renderer->triangleShader->setInt("drawSelection", 0);
        mesh->drawTriangleGraphics();
        renderer->lineShader->use();
        renderer->lineShader->setMat4("projectionView", projectionView);
        mesh->drawLineGraphics();

        auto buffer = new GLbyte[thumbnailSize * thumbnailSize * 3];
        glReadPixels(0, 0, thumbnailSize, thumbnailSize, GL_RGB, GL_UNSIGNED_BYTE, buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        images[ldrFile] = buffer;
    }
    lastAccessed.remove(ldrFile);
    lastAccessed.push_back(ldrFile);
    return images[ldrFile];
}

ThumbnailGenerator::ThumbnailGenerator(Renderer *renderer) : renderer(renderer), meshCollection(&renderer->meshCollection) {
    thumbnailSize = config::get_long(config::THUMBNAIL_SIZE);
    maxCachedThumbnails = config::get_long(config::THUMBNAIL_CACHE_SIZE_BYTES)/3/thumbnailSize/thumbnailSize;
    renderer->createFramebuffer(&framebuffer, &textureBuffer, &renderBuffer, thumbnailSize, thumbnailSize);
    projection = glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, 1000.0f);
}

void ThumbnailGenerator::discardOldestImages(int reserve_space_for) {
    while (lastAccessed.size()>maxCachedThumbnails-reserve_space_for) {
        auto lastAccessedIt = lastAccessed.front();
        delete [] images[lastAccessedIt];
        lastAccessed.remove(lastAccessedIt);
    }
}

void ThumbnailGenerator::cleanup() {
    Renderer::deleteFramebuffer(&framebuffer, &textureBuffer, &renderBuffer);
}
