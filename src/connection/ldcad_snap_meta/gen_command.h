#pragma once
#include "base.h"

namespace bricksim::connection::ldcad_snap_meta {
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
}