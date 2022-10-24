#include "clp_command.h"
#include "parse.h"
#include "write.h"

namespace bricksim::connection::ldcad_snap_meta {
    ClpCommand::ClpCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(parse::optionalStringParameter(parameters, "id")),
        pos(parse::optionalVec3Parameter(parameters, "pos")),
        ori(parse::optionalMat3Parameter(parameters, "ori")),
        radius(parse::floatParameter(parameters, "radius", 4.f)),
        length(parse::floatParameter(parameters, "length", 8.f)),
        center(parse::boolParameter(parameters, "center", false)),
        slide(parse::boolParameter(parameters, "slide", false)),
        scale(parse::enumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(parse::enumParameter(parameters, "mirror", MirrorType::NONE)) {
    }
    bool ClpCommand::operator==(const ClpCommand& rhs) const {
        return id == rhs.id
               && pos == rhs.pos
               && ori == rhs.ori
               && radius == rhs.radius
               && length == rhs.length
               && center == rhs.center
               && slide == rhs.slide
               && scale == rhs.scale
               && mirror == rhs.mirror;
    }
    bool ClpCommand::operator!=(const ClpCommand& rhs) const {
        return !(rhs == *this);
    }
    written_param_container ClpCommand::getParameters() const {
        written_param_container result;
        write::optionalStringParameter(result, "id", id);
        write::optionalVec3Parameter(result, "pos", pos);
        write::optionalMat3Parameter(result, "ori", ori);
        write::floatParameter(result, "radius", radius, 4.f);
        write::floatParameter(result, "length", length, 8.f);
        write::boolParameter(result, "center", center);
        write::boolParameter(result, "slide", slide);
        write::enumParameter(result, "scale", scale, ScaleType::NONE);
        write::enumParameter(result, "mirror", mirror, MirrorType::NONE);
        return result;
    }
    const char* ClpCommand::getName() const {
        return NAME;
    }
}