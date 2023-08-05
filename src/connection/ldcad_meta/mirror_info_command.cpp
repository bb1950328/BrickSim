#include "mirror_info_command.h"
#include "parse.h"
#include "write.h"
namespace bricksim::connection::ldcad_meta {

    MirrorInfoCommand::MirrorInfoCommand(const parsed_param_container& parameters) :
        MetaCommand(CommandType::MIRROR_INFO),
        baseFlip(parse::optionalEnumParameter<Axis>(parameters, "baseflip")),
        corOri(parse::optionalMat3Parameter(parameters, "corori")),
        counterPart(parse::optionalStringParameter(parameters, "counterpart")),
        posCor(parse::optionalVec3Parameter(parameters, "poscor")),
        inheritable(parse::optionalBoolParameter(parameters, "inheritable")) {
    }
    bool MirrorInfoCommand::operator==(const MirrorInfoCommand& rhs) const {
        return baseFlip == rhs.baseFlip
               && corOri == rhs.corOri
               && counterPart == rhs.counterPart
               && posCor == rhs.posCor
               && inheritable == rhs.inheritable;
    }
    written_param_container MirrorInfoCommand::getParameters() const {
        written_param_container result;
        write::optionalEnumParameter(result, "baseFlip", baseFlip);
        write::optionalMat3Parameter(result, "corOri", corOri);
        write::optionalStringParameter(result, "counterPart", counterPart);
        write::optionalVec3Parameter(result, "posCor", posCor);
        write::optionalBoolParameter(result, "inheritable", inheritable);
        return result;
    }
}
