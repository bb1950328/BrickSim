#include "incl_command.h"
#include "parse.h"
#include "write.h"

namespace bricksim::connection::ldcad_snap_meta {
    InclCommand::InclCommand(const parsed_param_container& parameters) :
        id(parse::optionalStringParameter(parameters, "id")),
        pos(parse::optionalVec3Parameter(parameters, "pos")),
        ori(parse::optionalMat3Parameter(parameters, "ori")),
        scale(parse::optionalVec3Parameter(parameters, "scale")),
        ref(parse::optionalStringParameter(parameters, "ref").value()),
        grid(parse::optionalGridParameter(parameters, "grid")) {
    }
    bool InclCommand::operator==(const InclCommand& rhs) const {
        return id == rhs.id
               && pos == rhs.pos
               && ori == rhs.ori
               && scale == rhs.scale
               && ref == rhs.ref
               && grid == rhs.grid;
    }

    written_param_container InclCommand::getParameters() const {
        written_param_container result;
        write::optionalStringParameter(result, "id", id);
        write::optionalVec3Parameter(result, "pos", pos);
        write::optionalMat3Parameter(result, "ori", ori);
        write::optionalVec3Parameter(result, "scale", scale);
        write::stringParameter(result, "ref", ref);
        write::optionalGridParameter(result, "grid", grid);
        return result;
    }
    const char* InclCommand::getName() const {
        return NAME;
    }
}
