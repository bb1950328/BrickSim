#pragma once

#include "../ldr/colors.h"
#include "../ldr/files.h"
#include "scene.h"
#include <list>
#include <memory>
#include "../helpers/util.h"

namespace bricksim::graphics {
    typedef std::pair<std::shared_ptr<ldr::File>, ldr::ColorReference> thumbnail_file_key_t;
}
namespace std {
    template<>
    struct hash<bricksim::graphics::thumbnail_file_key_t> {
        std::size_t operator()(const bricksim::graphics::thumbnail_file_key_t& value) const {
            return bricksim::util::combinedHash(value.first, value.second);
        }
    };
}

namespace bricksim::graphics {
    class ThumbnailGenerator {
    private:
        std::shared_ptr<Scene> scene;
        const std::shared_ptr<FitContentCamera> camera;
        uomap_t<thumbnail_file_key_t, unsigned int> images;
        std::list<thumbnail_file_key_t> lastAccessed;
        glm::mat4 projection;
        int maxCachedThumbnails;
        int framebufferSize = 0;
        [[nodiscard]] unsigned int copyImageToTexture() const;
        glm::vec3 renderedRotationDegrees;
        std::list<thumbnail_file_key_t> renderRequests;//TODO check if this can be made a set (maybe faster)
    public:
        int size;

        glm::vec3 rotationDegrees;
        ThumbnailGenerator();
        unsigned int getThumbnail(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color);
        std::optional<unsigned int> getThumbnailNonBlocking(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color);
        bool isThumbnailAvailable(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color);

        void discardOldestImages(int reserve_space_for = 1);
        void discardAllImages();

        bool workOnRenderQueue();
        bool renderQueueEmpty();
        void removeFromRenderQueue(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color);
    };
}
