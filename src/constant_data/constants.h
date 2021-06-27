#pragma once
#include "../types.h"
#include "../helpers/platform_detection.h"

namespace bricksim::constants {
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

    extern const uint16_t versionMajor;
    extern const uint16_t versionMinor;
    extern const uint16_t versionPatch;
    extern const char *versionString;

    extern const float totalWorkHours;
    extern const uint16_t gitCommitCount;
    extern const char *gitCommitHash;
}
