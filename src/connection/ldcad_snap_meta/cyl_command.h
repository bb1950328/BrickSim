#pragma once
#include "base.h"

namespace bricksim::connection::ldcad_snap_meta {
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
}