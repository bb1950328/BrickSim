#ifndef BRICKSIM_CONSTANTS_H
#define BRICKSIM_CONSTANTS_H

#include "../types.h"
#include "../helpers/platform_detection.h"

namespace constants {
    constexpr float LDU_TO_OPENGL_SCALE = 0.01f;
    extern const glm::mat4 LDU_TO_OPENGL_ROTATION;
    constexpr layer_t DEFAULT_LAYER=0;

    constexpr uint16_t versionMajor = BRICKSIM_VERSION_MAJOR;
    constexpr uint16_t versionMinor = BRICKSIM_VERSION_MINOR;
    constexpr uint16_t versionPatch = BRICKSIM_VERSION_PATCH;
    extern const char* versionString;

    constexpr float totalWorkHours = BRICKSIM_TOTAL_HOURS;
    constexpr uint16_t gitCommitCount = BRICKSIM_GIT_COMMIT_COUNT;
    extern const char* gitCommitHash;
}
#endif //BRICKSIM_CONSTANTS_H
