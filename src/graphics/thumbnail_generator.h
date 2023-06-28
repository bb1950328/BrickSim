#pragma once

#include "../config.h"
#include "../helpers/util.h"
#include "../ldr/colors.h"
#include "../ldr/files.h"
#include "scene.h"
#include <list>
#include <memory>

namespace bricksim::graphics {
    using thumbnail_file_key_t = std::pair<std::shared_ptr<ldr::File>, ldr::ColorReference>;
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
        const std::shared_ptr<FitContentCamera> camera = std::make_shared<FitContentCamera>();
        uomap_t<thumbnail_file_key_t, std::shared_ptr<Texture>> images;
        std::list<thumbnail_file_key_t> lastAccessed;
        glm::mat4 projection = glm::perspective(glm::radians(50.0f), 1.0f, 0.001f, 1000.0f);
        size_t maxCachedThumbnails;
        int framebufferSize = 0;
        [[nodiscard]] unsigned int copyImageToTexture() const;
        glm::vec3 renderedRotationDegrees;
        std::list<thumbnail_file_key_t> renderRequests;//TODO check if this can be made a set (maybe faster)
    public:
        int size = config::get(config::THUMBNAIL_SIZE);

        glm::vec3 rotationDegrees = glm::vec3(45, -45, 0);
        ThumbnailGenerator();
        std::shared_ptr<Texture> getThumbnail(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color);
        std::optional<std::shared_ptr<Texture>> getThumbnailNonBlocking(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color);
        bool isThumbnailAvailable(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color);

        void discardOldestImages(size_t reserve_space_for = 1);
        void discardAllImages();

        bool workOnRenderQueue();
        [[nodiscard]] bool renderQueueEmpty() const;
        void removeFromRenderQueue(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color);
    };
}
