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
    unsigned int framebuffer, textureBuffer, renderBuffer;
    int framebufferSize = 0;
    void discardOldestImages(int reserve_space_for=1);
public:
    int thumbnailSize;
    unsigned int getThumbnail(const LdrFile* ldrFile);
    void cleanup();

    ThumbnailGenerator(Renderer *renderer);
};

#endif //BRICKSIM_THUMBNAIL_GENERATOR_H
