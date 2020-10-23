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
    std::map<const LdrFile*, GLbyte*> images;
    std::list<const LdrFile*> lastAccessed;
    MeshCollection* meshCollection;
    Renderer* renderer;
    glm::mat4 projection;
    int thumbnailSize;
    int maxCachedThumbnails;

    unsigned int framebuffer, textureBuffer, renderBuffer;
    void discardOldestImages(int reserve_space_for=1);
public:
    GLbyte *getThumbnail(const LdrFile* ldrFile);
    void cleanup();

    ThumbnailGenerator(Renderer *renderer);
};

#endif //BRICKSIM_THUMBNAIL_GENERATOR_H
