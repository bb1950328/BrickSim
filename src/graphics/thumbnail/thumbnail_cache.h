#pragma once

#include "../texture.h"

#include <string>
#include "../../ldr/colors.h"

namespace bricksim::graphics {
    struct ThumbnailAtlasKey {
    };

    class ThumbnailAtlas {
        const unsigned int thumbnailSize;
        const unsigned int thumbnailsPerRow;
        //uomap_t<std::string, unsigned int> indices;
        std::shared_ptr<Texture> texture;

    public:
        explicit ThumbnailAtlas(unsigned int thumbnailSize);

        unsigned int capacity() const {
            return thumbnailsPerRow * thumbnailsPerRow;
        }
    };

    class ThumbnailCache {
    };
}
