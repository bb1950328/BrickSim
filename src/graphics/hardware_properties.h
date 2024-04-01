#pragma once

#include <glad/glad.h>

namespace bricksim::graphics {
    struct HardwareProperties {
        GLint maxTextureSize;
        const GLubyte *vendor;
        const GLubyte *renderer;
    };

    const HardwareProperties &getHardwareProperties();

    void initializeHardwareProperties();
}
