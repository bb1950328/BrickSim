#pragma once

#include "../types.h"
#include "glm/mat3x3.hpp"
#include "glm/vec3.hpp"
#include <optional>
#include <string>
#include <variant>
#include <vector>
namespace bricksim::connection::ldcad_snap_meta {
    //documentation: http://www.melkert.net/LDCad/tech/meta#:~:text=20%20and%2020.-,Part%20snapping%20metas,-Part%20snapping%20metas

    typedef uomap_t<std::string_view, std::string_view> parsed_param_container;
    struct ClearCommand {
        explicit ClearCommand(const parsed_param_container& parameters);
        std::optional<std::string> id;
    };
    struct Grid {
        bool centerX;
        bool centerZ;
        uint32_t countX;
        uint32_t countZ;
        float spacingX;
        float spacingZ;
        explicit Grid(std::string_view command);
    };
    struct InclCommand {
        explicit InclCommand(const parsed_param_container& parameters);
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
        explicit CylCommand(const parsed_param_container& parameters);
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
        explicit ClpCommand(const parsed_param_container& parameters);
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
        explicit FgrCommand(const parsed_param_container& parameters);
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

    typedef std::variant<BoundingPnt, BoundingBox, BoundingCube, BoundingCyl, BoundingSph> bounding_variant_t;
    struct GenCommand {
        explicit GenCommand(const parsed_param_container& parameters);
        std::optional<std::string> id;
        std::optional<std::string> group;
        std::optional<glm::vec3> pos;
        std::optional<glm::mat3> ori;
        Gender gender;
        bounding_variant_t bounding;
        ScaleType scale;
        MirrorType mirror;
    };

    typedef std::variant<std::monostate, ClearCommand, InclCommand, CylCommand, ClpCommand, FgrCommand, GenCommand> command_variant_t;
    class MetaLine {
    public:
        command_variant_t data;
        explicit MetaLine(command_variant_t  data);
    };

    class Reader {
    public:
        static MetaLine readLine(std::string_view line);
    };
}