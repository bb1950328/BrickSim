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

namespace bricksim::connection::ldcad_snap_meta {
    //documentation: http://www.melkert.net/LDCad/tech/meta#:~:text=20%20and%2020.-,Part%20snapping%20metas,-Part%20snapping%20metas

    typedef uomap_t<std::string_view, std::string_view> parsed_param_container;
    typedef uomap_t<std::string, std::string> written_param_container;

    struct Grid {
        bool centerX;
        bool centerZ;
        uint32_t countX;
        uint32_t countZ;
        float spacingX;
        float spacingZ;
        explicit Grid(std::string_view command);

        bool operator==(const Grid& rhs) const;
        bool operator!=(const Grid& rhs) const;
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
        bool operator==(const CylShapeBlock& rhs) const;
        bool operator!=(const CylShapeBlock& rhs) const;
    };
    enum class CylCaps {
        NONE,
        ONE,
        TWO,
        A,
        B,
    };

    class MetaCommand {
    public:
        [[nodiscard]] std::string to_string() const;

    protected:
        [[nodiscard]] virtual written_param_container getParameters() const = 0;
        [[nodiscard]] virtual const char* getName() const = 0;
    };

    class Reader {
    public:
        static std::shared_ptr<MetaCommand> readLine(std::string_view line);
    };
}