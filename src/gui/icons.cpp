#include "icons.h"
#include "../constant_data/resources.h"

namespace bricksim::gui::icons {
    void initialize() {
        //assert(magic_enum::enum_count<IconType>()==COUNT && "update icons::COUNT");
    }
    void cleanup() {
    }

    util::RawImage getRawImage(IconType icon, IconSize size) {
        return util::readImage(getPNG(icon, size));
    }
}
