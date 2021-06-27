#ifndef BRICKSIM_THUMBNAIL_GENERATOR_H
#define BRICKSIM_THUMBNAIL_GENERATOR_H

#include <memory>
#include <list>
#include "../ldr_files/ldr_files.h"
#include "../ldr_files/ldr_colors.h"
#include "scene.h"

namespace bricksim::graphics {
    class ThumbnailGenerator {
        typedef std::pair<std::shared_ptr<ldr::File>, ldr::ColorReference> file_key_t;
    private:
        std::shared_ptr<Scene> scene;
        const std::shared_ptr<FitContentCamera> camera;
        std::map<file_key_t, unsigned int> images;
        std::map<std::shared_ptr<const mesh::Mesh>, std::vector<float>> meshDimensions;
        std::list<file_key_t> lastAccessed;
        glm::mat4 projection;
        int maxCachedThumbnails;
        int framebufferSize = 0;
        [[nodiscard]] unsigned int copyImageToTexture() const;
        glm::vec3 renderedRotationDegrees;
        std::list<file_key_t> renderRequests;//TODO check if this can be made a set (maybe faster)
    public:
        int size;

        glm::vec3 rotationDegrees;
        ThumbnailGenerator();
        unsigned int getThumbnail(const std::shared_ptr<ldr::File> &ldrFile, ldr::ColorReference color);
        std::optional<unsigned int> getThumbnailNonBlocking(const std::shared_ptr<ldr::File> &ldrFile, ldr::ColorReference color);
        bool isThumbnailAvailable(const std::shared_ptr<ldr::File> &ldrFile, ldr::ColorReference color);

        void discardOldestImages(int reserve_space_for = 1);
        void discardAllImages();

        bool workOnRenderQueue();
        bool renderQueueEmpty();
        void removeFromRenderQueue(const std::shared_ptr<ldr::File> &ldrFile, ldr::ColorReference color);

    };
}
#endif //BRICKSIM_THUMBNAIL_GENERATOR_H
