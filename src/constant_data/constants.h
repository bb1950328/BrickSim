#pragma once
#include "../helpers/color.h"
#include "../types.h"

namespace bricksim::constants {
    constexpr float LDU_TO_OPENGL_SCALE = 0.01f;
    extern const glm::mat4 LDU_TO_OPENGL_ROTATION;
    extern const glm::mat4 LDU_TO_OPENGL;
    extern const glm::mat4 OPENGL_TO_LDU;
    constexpr layer_t DEFAULT_LAYER = 0;
    constexpr layer_t TRANSFORM_GIZMO_LAYER = 32;
    constexpr layer_t DEBUG_NODES_LAYER = 40;

    extern const char* const SEGMENT_PART_NAMES;

    constexpr mesh_identifier_t MESH_ID_ORIENTATION_CUBE_FIRST = 1;//6 sides
    constexpr mesh_identifier_t MESH_ID_ARROW = 7;
    constexpr mesh_identifier_t MESH_ID_UV_SPHERE = 10;
    constexpr mesh_identifier_t MESH_ID_QUARTER_TORUS = 11;
    constexpr mesh_identifier_t MESH_ID_TRANSFORM_GIZMO_2D_ARROW = 12;
    constexpr mesh_identifier_t MESH_ID_CUBE = 13;
    constexpr mesh_identifier_t MESH_ID_SELECTION_VISUALIZATION = 14;
    constexpr mesh_identifier_t MESH_ID_CYLINDER = 15;
    constexpr mesh_identifier_t MESH_ID_LINE_SUN = 16;
    constexpr mesh_identifier_t MESH_ID_XYZ_LINES = 17;
    constexpr mesh_identifier_t MESH_ID_POINT_MARKER = 18;
    constexpr mesh_identifier_t MESH_ID_LINE_UV_SPHERE = 19;
    constexpr mesh_identifier_t MESH_ID_LINE_CYLINDER = 20;
    constexpr mesh_identifier_t MESH_ID_LINE_BOX = 21;

    extern const uint16_t versionMajor;
    extern const uint16_t versionMinor;
    extern const uint16_t versionPatch;
    extern const char* versionString;

    extern const float totalWorkHours;
    extern const uint16_t gitCommitCount;
    extern const char* gitCommitHash;

    static constexpr auto LDRAW_LIBRARY_DOWNLOAD_URL = "https://library.ldraw.org/library/updates/complete.zip";
    static constexpr auto LDRAW_LIBRARY_UPDATES_XML_URL = "https://library.ldraw.org/updates?output=XML";

    static constexpr auto LDRAW_CONFIG_FILE_NAME = "LDConfig.ldr";

    constexpr float pInf = std::numeric_limits<float>::infinity();
    constexpr float nInf = -pInf;

    constexpr auto AXIS_COLORS = std::to_array({color::RED, color::GREEN, color::BLUE});

    static constexpr auto DISALLOWED_FILENAME_CHARS = "/\\<>:\"|?*&";

    static constexpr auto UTF8_BOM = "\xEF\xBB\xBF";
    static constexpr auto UTF16BE_BOM = "\xFE\xFF";
    static constexpr auto UTF16LE_BOM = "\xFF\xFE";
    static constexpr auto UTF32BE_BOM = "\x00\x00\xFE\xFF";
    static constexpr auto UTF32LE_BOM = "\xFF\xFE\x00\x00";
}
