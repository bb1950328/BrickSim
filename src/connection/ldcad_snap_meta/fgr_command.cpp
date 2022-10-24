#include "fgr_command.h"
#include "parse.h"
#include "write.h"

namespace bricksim::connection::ldcad_snap_meta {
    FgrCommand::FgrCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(parse::optionalStringParameter(parameters, "id")),
        group(parse::optionalStringParameter(parameters, "group")),
        genderOfs(parse::enumParameter(parameters, "genderOfs", Gender::M)),
        radius(parse::floatParameter(parameters, "radius", 0.f)),
        center(parse::boolParameter(parameters, "center", false)),
        scale(parse::enumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(parse::enumParameter(parameters, "mirror", MirrorType ::NONE)),
        pos(parse::optionalVec3Parameter(parameters, "pos")),
        ori(parse::optionalMat3Parameter(parameters, "ori")),
        seq(parse::floatVectorParameter(parameters, "seq")) {
    }
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
    bool FgrCommand::operator!=(const FgrCommand& rhs) const {
        return !(rhs == *this);
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
    const char* FgrCommand::getName() const {
        return NAME;
    }
}