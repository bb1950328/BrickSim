// thumbnail_generator.h
// Created by bab21 on 23.10.20.
//

#ifndef BRICKSIM_THUMBNAIL_GENERATOR_H
#define BRICKSIM_THUMBNAIL_GENERATOR_H

#include <map>
#include <list>
#include <queue>
#include "ldr_files.h"
#include "mesh_collection.h"
#include "renderer.h"

class ThumbnailGenerator {
private:
    std::map<std::pair<const LdrFile*, const LdrColor*>, unsigned int> images;
    std::map<const Mesh*, std::vector<float>> meshDimensions;
    std::list<std::pair<const LdrFile*, const LdrColor*>> lastAccessed;
    MeshCollection* meshCollection;
    Renderer* renderer;
    glm::mat4 projection;
    int maxCachedThumbnails;
    int framebufferSize = 0;
    [[nodiscard]] unsigned int copyFramebufferToTexture() const;
    glm::vec3 renderedRotationDegrees;
    std::list<std::pair<const LdrFile*, const LdrColor*>> renderRequests;
public:
    unsigned int framebuffer, textureBuffer, renderBuffer;
    int size;
    glm::vec3 rotationDegrees;
    unsigned int getThumbnail(const LdrFile* ldrFile, const LdrColor *color);
    std::optional<unsigned int> getThumbnailNonBlocking(const LdrFile *ldrFile, const LdrColor *color);
    void cleanup();
    void discardOldestImages(int reserve_space_for=1);
    explicit ThumbnailGenerator(Renderer *renderer);
    void discardAllImages();
    bool workOnRenderQueue();
    bool renderQueueEmpty();

    void saveFramebufferToBMP(const std::string &filename) const;
    void initialize();
};

#endif //BRICKSIM_THUMBNAIL_GENERATOR_H
