// thumbnail_generator.h
// Created by bab21 on 23.10.20.
//

#ifndef BRICKSIM_THUMBNAIL_GENERATOR_H
#define BRICKSIM_THUMBNAIL_GENERATOR_H

#include <map>

#include <list>
#include <queue>
#include "ldr_files/ldr_files.h"
#include "mesh_collection.h"
#include "renderer.h"
class ThumbnailGenerator {
    typedef std::pair<std::shared_ptr<LdrFile>, std::shared_ptr<const LdrColor>> file_key_t;
private:
    std::map<file_key_t, unsigned int> images;
    std::map<std::shared_ptr<const Mesh>, std::vector<float>> meshDimensions;
    std::list<file_key_t> lastAccessed;
    std::shared_ptr<MeshCollection> meshCollection;
    std::shared_ptr<Renderer> renderer;
    glm::mat4 projection;
    int maxCachedThumbnails;
    int framebufferSize = 0;
    [[nodiscard]] unsigned int copyFramebufferToTexture() const;
    glm::vec3 renderedRotationDegrees;
    std::list<file_key_t> renderRequests;//TODO check if this can be made a set (maybe faster)
public:
    unsigned int framebuffer, textureBuffer, renderBuffer;
    int size;
    glm::vec3 rotationDegrees;
    unsigned int getThumbnail(const std::shared_ptr<LdrFile>& ldrFile, const std::shared_ptr<const LdrColor>& color);
    std::optional<unsigned int> getThumbnailNonBlocking(const std::shared_ptr<LdrFile>& ldrFile, std::shared_ptr<const LdrColor> color);
    bool isThumbnailAvailable(const std::shared_ptr<LdrFile>& ldrFile, std::shared_ptr<const LdrColor> color);
    void cleanup();
    void discardOldestImages(int reserve_space_for=1);
    explicit ThumbnailGenerator(std::shared_ptr<Renderer> renderer, std::shared_ptr<MeshCollection> meshCollection);
    void discardAllImages();
    bool workOnRenderQueue();
    bool renderQueueEmpty();
    void removeFromRenderQueue(const std::shared_ptr<LdrFile>& ldrFile, std::shared_ptr<const LdrColor> color);

    void saveFramebufferToBMP(const std::string &filename) const;
    void initialize();
};

#endif //BRICKSIM_THUMBNAIL_GENERATOR_H
