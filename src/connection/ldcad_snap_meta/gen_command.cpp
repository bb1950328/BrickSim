#include "gen_command.h"
#include "parse.h"
#include "write.h"

namespace bricksim::connection::ldcad_snap_meta {
    GenCommand::GenCommand(const parsed_param_container& parameters) :
        id(parse::optionalStringParameter(parameters, "id")),
        group(parse::optionalStringParameter(parameters, "group")),
        pos(parse::optionalVec3Parameter(parameters, "pos")),
        ori(parse::optionalMat3Parameter(parameters, "ori")),
        gender(parse::enumParameter(parameters, "gender", Gender::M)),
        bounding(parse::boundingParameter(parameters, "bounding")),
        scale(parse::enumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(parse::enumParameter(parameters, "mirror", MirrorType::NONE)) {
    }
    bool GenCommand::operator==(const GenCommand& rhs) const {
        return id == rhs.id
               && group == rhs.group
               && pos == rhs.pos
               && ori == rhs.ori
               && gender == rhs.gender
               && bounding == rhs.bounding
               && scale == rhs.scale
               && mirror == rhs.mirror;
    }

    written_param_container GenCommand::getParameters() const {
        written_param_container result;
        write::optionalStringParameter(result, "id", id);
        write::optionalStringParameter(result, "group", group);
        write::optionalVec3Parameter(result, "pos", pos);
        write::optionalMat3Parameter(result, "ori", ori);
        write::enumParameter(result, "gender", gender, Gender::M);
        write::boundingParameter(result, "bounding", bounding);
        write::enumParameter(result, "scale", scale, ScaleType::NONE);
        write::enumParameter(result, "mirror", mirror, MirrorType::NONE);
        return result;
    }
    const char* GenCommand::getName() const {
        return NAME;
    }
}
