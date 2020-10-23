// thumbnail_generator.cpp
// Created by bab21 on 23.10.20.
//

#include <glm/gtx/string_cast.hpp>
#include "thumbnail_generator.h"
#include "config.h"
#include "ldr_colors.h"

unsigned int ThumbnailGenerator::getThumbnail(const LdrFile* ldrFile) {
    auto imgIt = images.find(ldrFile);
    if (imgIt == images.end()) {
        if (framebufferSize!=thumbnailSize) {
            renderer->createFramebuffer(&framebuffer, &textureBuffer, &renderBuffer, thumbnailSize, thumbnailSize);
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

        float xMin, xMax, yMin, yMax, zMin, zMax;
        auto meshDimIt = meshDimensions.find(mesh);
        if (meshDimIt!=meshDimensions.end()) {
            xMin = meshDimIt->second[0];
            xMax = meshDimIt->second[1];
            yMin = meshDimIt->second[2];
            yMax = meshDimIt->second[3];
            zMin = meshDimIt->second[4];
            zMax = meshDimIt->second[5];
        } else {
            const auto firstPos = mesh->triangleVertices.begin()->second->begin()->position;
            xMin = xMax = firstPos.x;
            yMin = yMax = firstPos.y;
            zMin = zMax = firstPos.z;
            for (const auto &pair : mesh->triangleVertices) {
                for (const auto &vertex : *pair.second) {
                    xMin = std::min(xMin, vertex.position.x);
                    xMax = std::max(xMax, vertex.position.x);
                    yMin = std::min(yMin, vertex.position.y);
                    yMax = std::max(yMax, vertex.position.y);
                    zMin = std::min(zMin, vertex.position.z);
                    zMax = std::max(zMax, vertex.position.z);
                }
            }
            meshDimensions[mesh] = {xMin, xMax, yMin, yMax, zMin, zMax};
        }

        glm::vec3 minPos = mesh->globalModel*glm::vec4(xMin, yMin, zMin, 1.0f);
        glm::vec3 maxPos = mesh->globalModel*glm::vec4(xMax, yMax, zMax, 1.0f);
        MeshInstance tmpInstance{
                LdrColorRepository::getInstance()->get_color(1),
                glm::mat4(1.0f),
                0
        };
        mesh->instances.push_back(tmpInstance);
        mesh->instancesHaveChanged = true;
        mesh->writeGraphicsData();
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST); // todo check if this is needed
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto center = glm::vec4((minPos+maxPos)/2.0f, 1.0);
        std::cout << glm::to_string(minPos) << " ... " << glm::to_string(center) << " ... " << glm::to_string(maxPos) << std::endl;
        auto meshDiameter = glm::distance(minPos, maxPos)*1.3;
        std::cout << meshDiameter << std::endl;
        auto viewPos = (glm::vec4(meshDiameter, 0, 0, 1))+center;//todo make mesh centered on framebuffer
        auto view = glm::lookAt(glm::vec3(viewPos), glm::vec3(/*center*/0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 &projectionView = projection * view;
        renderer->triangleShader->use();
        renderer->triangleShader->setVec3("viewPos", viewPos);
        renderer->triangleShader->setMat4("projectionView", projectionView);
        renderer->triangleShader->setInt("drawSelection", 0);
        mesh->drawTriangleGraphics();
        renderer->lineShader->use();
        renderer->lineShader->setMat4("projectionView", projectionView);
        mesh->drawLineGraphics();

        GLbyte buffer[thumbnailSize * thumbnailSize * 3];
        glReadPixels(0, 0, thumbnailSize, thumbnailSize, GL_RGB, GL_UNSIGNED_BYTE, buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        unsigned int textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, thumbnailSize, thumbnailSize, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
        images[ldrFile] = textureId;
        mesh->instances = instanceBackup;
        mesh->instancesHaveChanged = true;
    }
    lastAccessed.remove(ldrFile);
    lastAccessed.push_back(ldrFile);
    return images[ldrFile];
}

ThumbnailGenerator::ThumbnailGenerator(Renderer *renderer) : renderer(renderer), meshCollection(&renderer->meshCollection) {
    thumbnailSize = config::get_long(config::THUMBNAIL_SIZE);
    maxCachedThumbnails = config::get_long(config::THUMBNAIL_CACHE_SIZE_BYTES)/3/thumbnailSize/thumbnailSize;
    projection = glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, 1000.0f);
}

void ThumbnailGenerator::discardOldestImages(int reserve_space_for) {
    while (lastAccessed.size()>maxCachedThumbnails-reserve_space_for) {
        auto lastAccessedIt = lastAccessed.front();
        glDeleteTextures(1, &images[lastAccessedIt]);
        lastAccessed.remove(lastAccessedIt);
    }
}

void ThumbnailGenerator::cleanup() {
    Renderer::deleteFramebuffer(&framebuffer, &textureBuffer, &renderBuffer);
}
