#pragma once
#include "base.h"
#include "../bounding.h"

namespace bricksim::connection::ldcad_meta {
    class GenCommand : public MetaCommand {
    public:
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

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
    };
}
