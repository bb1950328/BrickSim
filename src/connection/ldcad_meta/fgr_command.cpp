#include "fgr_command.h"
#include "parse.h"
#include "write.h"

namespace bricksim::connection::ldcad_meta {
    FgrCommand::FgrCommand(const parsed_param_container& parameters) :
        MetaCommand(CommandType::SNAP_FGR),
        id(parse::optionalStringParameter(parameters, "id")),
        group(parse::optionalStringParameter(parameters, "group")),
        pos(parse::optionalVec3Parameter(parameters, "pos")),
        ori(parse::optionalMat3Parameter(parameters, "ori")),
        genderOfs(parse::enumParameter(parameters, "genderofs", Gender::M)),
        seq(parse::floatVectorParameter(parameters, "seq")),
        radius(parse::floatParameter(parameters, "radius", 0.f)),
        center(parse::boolParameter(parameters, "center", true)),//todo per the documentation the default is false but when trying in LDCad, the default is true
        scale(parse::enumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(parse::enumParameter(parameters, "mirror", MirrorType::NONE)) {}

    bool FgrCommand::operator==(const FgrCommand& rhs) const {
        return id == rhs.id
               && group == rhs.group
               && pos == rhs.pos
               && ori == rhs.ori
               && genderOfs == rhs.genderOfs
               && seq == rhs.seq
               && radius == rhs.radius
               && center == rhs.center
               && scale == rhs.scale
               && mirror == rhs.mirror;
    }

    written_param_container FgrCommand::getParameters() const {
        written_param_container result;
        write::optionalStringParameter(result, "id", id);
        write::optionalStringParameter(result, "group", group);
        write::optionalVec3Parameter(result, "pos", pos);
        write::optionalMat3Parameter(result, "ori", ori);
        write::enumParameter(result, "genderOfs", genderOfs, Gender::M);
        write::floatVectorParameter(result, "seq", seq);
        write::floatParameter(result, "radius", radius);
        write::boolParameter(result, "center", center);
        write::enumParameter(result, "scale", scale, ScaleType::NONE);
        write::enumParameter(result, "mirror", mirror, MirrorType::NONE);
        return result;
    }
}
