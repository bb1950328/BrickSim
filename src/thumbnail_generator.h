// thumbnail_generator.h
// Created by bab21 on 23.10.20.
//

#ifndef BRICKSIM_THUMBNAIL_GENERATOR_H
#define BRICKSIM_THUMBNAIL_GENERATOR_H

#include <map>
#include <list>
#include "ldr_files.h"
#include "mesh_collection.h"
#include "renderer.h"

class ThumbnailGenerator {
private:
    std::map<const LdrFile*, unsigned int> images;
    std::map<const Mesh*, std::vector<float>> meshDimensions;
    std::list<const LdrFile*> lastAccessed;
    MeshCollection* meshCollection;
    Renderer* renderer;
    glm::mat4 projection;
    int maxCachedThumbnails;
    int framebufferSize = 0;
    void discardOldestImages(int reserve_space_for=1);
    [[nodiscard]] unsigned int copyFramebufferToTexture() const;
public:
    unsigned int framebuffer, textureBuffer, renderBuffer;
    int size;
    glm::vec3 rotationDegrees;
    unsigned int getThumbnail(const LdrFile* ldrFile);

    void cleanup();

    explicit ThumbnailGenerator(Renderer *renderer);
};

#endif //BRICKSIM_THUMBNAIL_GENERATOR_H
