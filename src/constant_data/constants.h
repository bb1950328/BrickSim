#ifndef BRICKSIM_CONSTANTS_H
#define BRICKSIM_CONSTANTS_H
#include "../types.h"
#include "../helpers/platform_detection.h"

namespace constants {
    constexpr float LDU_TO_OPENGL_SCALE = 0.01f;
    extern const glm::mat4 LDU_TO_OPENGL_ROTATION;
    extern const glm::mat4 LDU_TO_OPENGL;
    extern const glm::mat4 OPENGL_TO_LDU;
    constexpr layer_t DEFAULT_LAYER = 0;
    constexpr layer_t TRANSFORM_GIZMO_LAYER = 32;

    constexpr mesh_identifier_t MESH_ID_ORIENTATION_CUBE_FIRST = 1;//6 sides
    constexpr mesh_identifier_t MESH_ID_ARROW = 7;
    constexpr mesh_identifier_t MESH_ID_UV_SPHERE = 10;
    constexpr mesh_identifier_t MESH_ID_QUARTER_TORUS = 11;
    constexpr mesh_identifier_t MESH_ID_TRANSFORM_GIZMO_2D_ARROW = 12;
    constexpr mesh_identifier_t MESH_ID_CUBE = 12;

    constexpr uint16_t versionMajor =
#ifdef BRICKSIM_VERSION_MAJOR
            BRICKSIM_VERSION_MAJOR;
#else
#warning Please set the BRICKSIM_VERSION_MAJOR macro
    0;
#endif
    constexpr uint16_t versionMinor =
#ifdef BRICKSIM_VERSION_MINOR
            BRICKSIM_VERSION_MINOR;
#else
    #warning Please set the BRICKSIM_VERSION_MINOR macro
    0;
#endif
    ;
    constexpr uint16_t versionPatch =
#ifdef BRICKSIM_VERSION_PATCH
            BRICKSIM_VERSION_PATCH;
#else
    #warning Please set the BRICKSIM_VERSION_PATCH macro
    0;
#endif
    ;
    extern const char *versionString;

    constexpr float totalWorkHours =
#ifdef BRICKSIM_TOTAL_HOURS
            BRICKSIM_TOTAL_HOURS;
#else
    #warning Please set the BRICKSIM_TOTAL_HOURS macro
    0;
#endif
    ;
    constexpr uint16_t gitCommitCount =
#ifdef BRICKSIM_GIT_COMMIT_COUNT
            BRICKSIM_GIT_COMMIT_COUNT;
#else
    #warning Please set the BRICKSIM_GIT_COMMIT_COUNT macro
    0;
#endif
    ;
    extern const char *gitCommitHash;
}
#endif //BRICKSIM_CONSTANTS_H
