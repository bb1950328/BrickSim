#include "thumbnail_cache.h"
#include "../hardware_properties.h"

namespace bricksim::graphics {
    ThumbnailAtlas::ThumbnailAtlas(const unsigned int thumbnailSize) :
            thumbnailSize(thumbnailSize), thumbnailsPerRow(getHardwareProperties().maxTextureSize / thumbnailSize) {
        texture = std::make_shared<Texture>(thumbnailSize * thumbnailsPerRow, thumbnailSize * thumbnailsPerRow, 3);
    }
}
