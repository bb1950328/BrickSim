#include "clear_command.h"
#include "parse.h"
#include "write.h"

namespace bricksim::connection::ldcad_meta {
    ClearCommand::ClearCommand(const parsed_param_container& parameters) :
        MetaCommand(CommandType::SNAP_CLEAR),
        id(parse::optionalStringParameter(parameters, "id")) {
    }
    bool ClearCommand::operator==(const ClearCommand& rhs) const {
        return id == rhs.id;
    }
    written_param_container ClearCommand::getParameters() const {
        written_param_container result;
        write::optionalStringParameter(result, "id", id);
        return result;
    }
}
