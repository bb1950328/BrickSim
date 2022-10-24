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
    typedef uomap_t<std::string, std::string> written_param_container;

    class MetaCommand {
    public:
        [[nodiscard]] std::string to_string() const;

    protected:
        [[nodiscard]] virtual written_param_container getParameters() const = 0;
        [[nodiscard]] virtual const char* getName() const = 0;
    };

    class ClearCommand : public MetaCommand {
    public:
        constexpr const static char* const NAME = "SNAP_CLEAR";
        explicit ClearCommand(const parsed_param_container& parameters);
        std::optional<std::string> id;

        bool operator==(const ClearCommand& rhs) const;
        bool operator!=(const ClearCommand& rhs) const;

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
    };

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

    class InclCommand : public MetaCommand {
    public:
        constexpr const static char* const NAME = "SNAP_INCL";
        explicit InclCommand(const parsed_param_container& parameters);
        std::optional<std::string> id;
        std::optional<glm::vec3> pos;
        std::optional<glm::mat3> ori;
        std::optional<glm::vec3> scale;
        std::string ref;
        std::optional<Grid> grid;

        bool operator==(const InclCommand& rhs) const;
        bool operator!=(const InclCommand& rhs) const;

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
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
    class CylCommand : public MetaCommand {
    public:
        constexpr const static char* const NAME = "SNAP_CYL";
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

        bool operator==(const CylCommand& rhs) const;
        bool operator!=(const CylCommand& rhs) const;

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
    };

    class ClpCommand : public MetaCommand {
    public:
        constexpr const static char* const NAME = "SNAP_CLP";
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

        bool operator==(const ClpCommand& rhs) const;
        bool operator!=(const ClpCommand& rhs) const;

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
    };

    class FgrCommand : public MetaCommand {
    public:
        constexpr const static char* const NAME = "SNAP_FGR";
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

        bool operator==(const FgrCommand& rhs) const;
        bool operator!=(const FgrCommand& rhs) const;

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
    };

    struct BoundingPnt {
        bool operator==(const BoundingPnt& rhs) const;
        bool operator!=(const BoundingPnt& rhs) const;
    };
    struct BoundingBox {
        float x;
        float y;
        float z;

        bool operator==(const BoundingBox& rhs) const;
        bool operator!=(const BoundingBox& rhs) const;
    };
    struct BoundingCube {
        float size;

        bool operator==(const BoundingCube& rhs) const;
        bool operator!=(const BoundingCube& rhs) const;
    };
    struct BoundingCyl {
        float radius;
        float length;

        bool operator==(const BoundingCyl& rhs) const;
        bool operator!=(const BoundingCyl& rhs) const;
    };
    struct BoundingSph {
        float radius;

        bool operator==(const BoundingSph& rhs) const;
        bool operator!=(const BoundingSph& rhs) const;
    };

    typedef std::variant<BoundingPnt, BoundingBox, BoundingCube, BoundingCyl, BoundingSph> bounding_variant_t;
    class GenCommand : public MetaCommand {
    public:
        constexpr const static char* const NAME = "SNAP_GEN";
        explicit GenCommand(const parsed_param_container& parameters);
        std::optional<std::string> id;
        std::optional<std::string> group;
        std::optional<glm::vec3> pos;
        std::optional<glm::mat3> ori;
        Gender gender;
        bounding_variant_t bounding;
        ScaleType scale;
        MirrorType mirror;

        bool operator==(const GenCommand& rhs) const;
        bool operator!=(const GenCommand& rhs) const;

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
    };

    class Reader {
    public:
        static std::shared_ptr<MetaCommand> readLine(std::string_view line);
    };
}