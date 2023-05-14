#pragma once
#include "base.h"

namespace bricksim::connection::ldcad_meta {
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

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
    };
}
