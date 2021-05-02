#ifndef BRICKSIM_CONSTANTS_H
#define BRICKSIM_CONSTANTS_H

#include "../types.h"
#include "../helpers/platform_detection.h"

namespace constants {
    constexpr float LDU_TO_OPENGL_SCALE = 0.01f;
    extern const glm::mat4 LDU_TO_OPENGL_ROTATION;
    constexpr layer_t DEFAULT_LAYER = 0;
    constexpr layer_t TRANSFORM_GIZMO_LAYER = 32;

    constexpr mesh_identifier_t MESH_ID_ORIENTATION_CUBE_FIRST = 1;//6 sides
    constexpr mesh_identifier_t MESH_ID_ARROW = 7;
    constexpr mesh_identifier_t MESH_ID_UV_SPHERE = 10;
    constexpr mesh_identifier_t MESH_ID_QUARTER_TORUS = 11;

    constexpr uint16_t versionMajor = BRICKSIM_VERSION_MAJOR;
    constexpr uint16_t versionMinor = BRICKSIM_VERSION_MINOR;
    constexpr uint16_t versionPatch = BRICKSIM_VERSION_PATCH;
    extern const char *versionString;

    constexpr float totalWorkHours = BRICKSIM_TOTAL_HOURS;
    constexpr uint16_t gitCommitCount = BRICKSIM_GIT_COMMIT_COUNT;
    extern const char *gitCommitHash;
}
#endif //BRICKSIM_CONSTANTS_H
