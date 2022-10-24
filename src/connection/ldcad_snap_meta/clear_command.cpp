#include "clear_command.h"
#include "parse.h"
#include "write.h"

namespace bricksim::connection::ldcad_snap_meta {
    ClearCommand::ClearCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(parse::optionalStringParameter(parameters, "id")) {
    }
    bool ClearCommand::operator==(const ClearCommand& rhs) const {
        return id == rhs.id;
    }
    bool ClearCommand::operator!=(const ClearCommand& rhs) const {
        return !(rhs == *this);
    }
    written_param_container ClearCommand::getParameters() const {
        written_param_container result;
        write::optionalStringParameter(result, "id", id);
        return result;
    }
    const char* ClearCommand::getName() const {
        return NAME;
    }
}