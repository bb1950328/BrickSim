#pragma once

#include "../../helpers/stringutil.h"
#include "../../types.h"
#include "../bounding.h"
#include "glm/mat3x3.hpp"
#include "glm/vec3.hpp"
#include <charconv>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace bricksim::connection::ldcad_meta {
    //documentation: http://www.melkert.net/LDCad/tech/meta#:~:text=20%20and%2020.-,Part%20snapping%20metas,-Part%20snapping%20metas

    using parsed_param_container = uomap_t<std::string, std::string_view>;
    using written_param_container = uomap_t<std::string, std::string>;

    struct Grid {
        bool centerX;
        bool centerZ;
        uint32_t countX;
        uint32_t countZ;
        float spacingX;
        float spacingZ;
        explicit Grid(std::string_view command);

        bool operator==(const Grid& rhs) const = default;
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
        bool operator==(const CylShapeBlock& rhs) const = default;
    };

    enum class CylCaps {
        NONE,
        ONE,
        TWO,
        A,
        B,
    };

    enum class Axis {
        X,
        Y,
        Z,
    };

    enum class CommandType {
        SNAP_CLEAR,
        SNAP_CLP,
        SNAP_CYL,
        SNAP_FGR,
        SNAP_GEN,
        SNAP_INCL,
        MIRROR_INFO,
    };

    class MetaCommand {
    public:
        [[nodiscard]] std::string to_string() const;
        virtual ~MetaCommand();
        const CommandType type;

    protected:
        [[nodiscard]] virtual written_param_container getParameters() const = 0;
        [[nodiscard]] const std::string_view getName() const;
        explicit MetaCommand(const CommandType type);
    };

    class Reader {
    public:
        static std::shared_ptr<MetaCommand> readLine(std::string_view line);
        static bool isUnsupportedCommand(std::string_view command);
    };
}
