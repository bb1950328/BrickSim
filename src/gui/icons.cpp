#include "icons.h"
#include "../constant_data/resources.h"

namespace bricksim::gui::icons {
    void initialize() {
        //assert(magic_enum::enum_count<IconType>()==COUNT && "update icons::COUNT");
    }

    void cleanup() {}

    util::RawImage getRawImage(IconType icon, IconSize size) {
        return util::readImage(getPNG(icon, size));
    }

    glm::vec2 getHotPoint(IconType icon) {
        switch (icon) {
            case Select:
            case SelectConnected:
            case SelectStronglyConnected:
                return {.38f, .2f};
            default:
                return {.5f, .5f};
        }
    }

    glm::ivec2 getHotPoint(IconType icon, IconSize size) {
        return getHotPoint(icon) * static_cast<float>(size);
    }
}
