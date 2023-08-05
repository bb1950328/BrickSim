#pragma once
#include "base.h"

namespace bricksim::connection::ldcad_meta {
    class ClearCommand : public MetaCommand {
    public:
        explicit ClearCommand(const parsed_param_container& parameters);
        std::optional<std::string> id;

        bool operator==(const ClearCommand& rhs) const;

    protected:
        [[nodiscard]] written_param_container getParameters() const override;
    };
}
