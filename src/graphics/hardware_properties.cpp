#include "hardware_properties.h"

namespace bricksim::graphics {
    namespace {
        HardwareProperties data;
    }

    const HardwareProperties &getHardwareProperties() {
        return data;
    }

    void initializeHardwareProperties() {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &data.maxTextureSize);
        data.vendor = glGetString(GL_VENDOR);
        data.renderer = glGetString(GL_RENDERER);
    }
}
