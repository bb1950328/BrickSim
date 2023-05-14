#pragma once
#include "base.h"

namespace bricksim::connection::ldcad_meta {
    /**
     * Documentation: https://forums.ldraw.org/thread-24323-post-39797.html#pid39797
     */
    class MirrorInfoCommand : public MetaCommand {
    public:
        constexpr const static char* const NAME = "MIRROR_INFO";
        explicit MirrorInfoCommand(const parsed_param_container& parameters);
        std::optional<Axis> baseFlip;
        std::optional<glm::mat3> corOri;
        std::optional<std::string> counterPart;
        std::optional<glm::vec3> posCor;
        std::optional<bool> inheritable;

        bool operator==(const MirrorInfoCommand& rhs) const;

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
        [[nodiscard]] const char* getName() const override;
    };
}
