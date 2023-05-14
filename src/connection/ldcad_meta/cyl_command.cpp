#include "cyl_command.h"
#include "parse.h"
#include "write.h"

namespace bricksim::connection::ldcad_meta {
    CylCommand::CylCommand(const parsed_param_container& parameters) :
        id(parse::optionalStringParameter(parameters, "id")),
        group(parse::optionalStringParameter(parameters, "group")),
        pos(parse::optionalVec3Parameter(parameters, "pos")),
        ori(parse::optionalMat3Parameter(parameters, "ori")),
        scale(parse::enumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(parse::enumParameter(parameters, "mirror", MirrorType::COR)),
        gender(parse::enumParameter(parameters, "gender", Gender::M)),
        secs(parse::cylShapeBlockParameter(parameters, "secs")),
        caps(parse::enumParameter(parameters, "caps", CylCaps::ONE)),
        grid(parse::optionalGridParameter(parameters, "grid")),
        center(parse::boolParameter(parameters, "center", false)),
        slide(parse::boolParameter(parameters, "slide", false)) {
    }
    bool CylCommand::operator==(const CylCommand& rhs) const {
        return id == rhs.id
               && group == rhs.group
               && pos == rhs.pos
               && ori == rhs.ori
               && scale == rhs.scale
               && mirror == rhs.mirror
               && gender == rhs.gender
               && secs == rhs.secs
               && caps == rhs.caps
               && grid == rhs.grid
               && center == rhs.center
               && slide == rhs.slide;
    }

    written_param_container CylCommand::getParameters() const {
        written_param_container result;
        write::optionalStringParameter(result, "id", id);
        write::optionalStringParameter(result, "group", group);
        write::optionalVec3Parameter(result, "pos", pos);
        write::optionalMat3Parameter(result, "ori", ori);
        write::enumParameter(result, "scale", scale, ScaleType::NONE);
        write::enumParameter(result, "mirror", mirror, MirrorType::COR);
        write::enumParameter(result, "gender", gender, Gender::M);
        write::cylShapeBlockParameter(result, "secs", secs);
        write::enumParameter(result, "caps", caps, CylCaps::ONE);
        write::optionalGridParameter(result, "grid", grid);
        write::boolParameter(result, "center", center, false);
        write::boolParameter(result, "slide", slide, false);
        return result;
    }
    const char* CylCommand::getName() const {
        return NAME;
    }
}
