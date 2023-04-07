#pragma once
#include "base.h"

namespace bricksim::connection::ldcad_snap_meta {
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

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
    };
}