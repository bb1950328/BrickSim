#pragma once
#include "base.h"

namespace bricksim::connection::ldcad_meta {
    class FgrCommand : public MetaCommand {
    public:
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

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
    };
}
