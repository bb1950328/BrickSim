#pragma once

#include "glm/mat3x3.hpp"
#include "glm/vec3.hpp"
#include <optional>
#include <string>
#include <variant>
#include <vector>
namespace bricksim::connection::ldcad_snap_meta {
    //documentation: http://www.melkert.net/LDCad/tech/meta#:~:text=20%20and%2020.-,Part%20snapping%20metas,-Part%20snapping%20metas

    struct ClearCommand {
        std::optional<std::string> id;
    };
    struct Grid {
        bool centerX;
        bool centerZ;
        uint32_t countX;
        uint32_t countZ;
        float spacingX;
        float spacingZ;
        Grid(std::string_view command);
    };
    struct InclCommand {
        std::optional<std::string> id;
        std::optional<glm::vec3> pos;
        std::optional<glm::mat3> ori;
        std::optional<glm::vec3> scale;
        std::string ref;
        std::optional<Grid> grid;
    };
    enum class ScaleType {
        NONE,
        YONLY,
        RONLY,
        YANDR,
    };
    enum class MirrorType {
        NONE,
        COR,
    };
    enum class Gender {
        M,
        F,
    };
    enum class CylShapeVariant {
        R,
        A,
        S,
        _L,
        L_,
    };
    struct CylShapeBlock {
        CylShapeVariant variant;
        float radius;
        float length;
    };
    enum class CylCaps {
        NONE,
        ONE,
        TWO,
        A,
        B,
    };
    struct CylCommand {
        std::optional<std::string> id;
        std::optional<std::string> group;
        std::optional<glm::vec3> pos;
        std::optional<glm::mat3> ori;
        ScaleType scale;
        MirrorType mirror;
        Gender gender;
        std::vector<CylShapeBlock> secs;
        CylCaps caps;
        std::optional<Grid> grid;
        bool center;
        bool slide;
    };

    struct ClpCommand {
        std::optional<std::string> id;
        std::optional<glm::vec3> pos;
        std::optional<glm::mat3> ori;
        float radius;
        float length;
        bool center;
        bool slide;
        ScaleType scale;
        MirrorType mirror;
    };

    struct FgrCommand {
        std::optional<std::string> id;
        std::optional<std::string> group;
        std::optional<glm::vec3> pos;
        std::optional<glm::mat3> ori;
        Gender genderOfs;
        std::vector<float> seq;
        float radius;
        bool center;
        ScaleType scale;
        MirrorType mirror;
    };

    struct BoundingPnt {
    };
    struct BoundingBox {
        float x;
        float y;
        float z;
    };
    struct BoundingCube {
        float size;
    };
    struct BoundingCyl {
        float radius;
        float length;
    };
    struct BoundingSph {
        float radius;
    };

    struct GenCommand {
        std::optional<std::string> id;
        std::optional<std::string> group;
        std::optional<glm::vec3> pos;
        std::optional<glm::mat3> ori;
        Gender gender;
        std::variant<BoundingPnt, BoundingBox, BoundingCube, BoundingCyl, BoundingSph> bounding;
        ScaleType scale;
        MirrorType mirror;
    };

    class MetaLine {
    public:
        const std::variant<ClearCommand, InclCommand, CylCommand, ClpCommand, FgrCommand, GenCommand> data;
    };

    class Reader {
        static MetaLine readLine(std::string_view line);
    };
}