#include "base.h"
#include "../../helpers/util.h"
#include "../../types.h"
#include "clear_command.h"
#include "clp_command.h"
#include "cyl_command.h"
#include "fast_float/fast_float.h"
#include "fgr_command.h"
#include "gen_command.h"
#include "glm/gtc/type_ptr.hpp"
#include "incl_command.h"
#include "parse.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h"
#include <utility>
#include <vector>

namespace bricksim::connection::ldcad_snap_meta {

    Grid::Grid(std::string_view command) {
        std::vector<std::string_view> words = stringutil::splitByChar(command, ' ');

        int i = 0;
        if (words[i] == "C") {
            centerX = true;
            ++i;
        } else {
            centerX = false;
        }

        parse::intFromString(words[i], countX);
        ++i;
        if (words[i] == "C") {
            centerZ = true;
            ++i;
        } else {
            centerZ = false;
        }

        parse::intFromString(words[i], countZ);
        ++i;

        parse::floatFromString(words[i], spacingX);
        ++i;

        parse::floatFromString(words[i], spacingZ);
        ++i;
    }

    std::shared_ptr<MetaCommand> Reader::readLine(std::string_view line) {
        parsed_param_container parameters;

        std::size_t end = 0;
        while (true) {
            const auto start = line.find('[', end);
            if (start == std::string_view::npos) {
                break;
            }
            const auto middle = line.find('=', start);
            end = line.find(']', start);
            const std::string_view key = stringutil::trim(line.substr(start + 1, middle - start - 1));
            const std::string_view value = stringutil::trim(line.substr(middle + 1, end - middle - 1));
            parameters.insert({stringutil::asLower(key), value});
        }
        if (line.starts_with(ClearCommand::NAME)) {
            return std::make_shared<ClearCommand>(parameters);
        } else if (line.starts_with(InclCommand::NAME)) {
            return std::make_shared<InclCommand>(parameters);
        } else if (line.starts_with(CylCommand::NAME)) {
            return std::make_shared<CylCommand>(parameters);
        } else if (line.starts_with(ClpCommand::NAME)) {
            return std::make_shared<ClpCommand>(parameters);
        } else if (line.starts_with(FgrCommand::NAME)) {
            return std::make_shared<FgrCommand>(parameters);
        } else if (line.starts_with(GenCommand::NAME)) {
            return std::make_shared<GenCommand>(parameters);
        } else {
            return nullptr;
        }
    }

    bool CylShapeBlock::operator==(const CylShapeBlock& rhs) const = default;

    std::string MetaCommand::to_string() const {
        std::string result(getName());
        for (const auto& param: getParameters()) {
            result.push_back(' ');
            result.push_back('[');
            result.append(param.first);
            result.push_back('=');
            result.append(param.second);
            result.push_back(']');
        }
        return result;
    }
    MetaCommand::~MetaCommand() = default;
}
