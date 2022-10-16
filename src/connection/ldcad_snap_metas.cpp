#include "ldcad_snap_metas.h"
#include "../helpers/stringutil.h"
#include "../types.h"
#include <charconv>
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

        std::from_chars(words[i].begin(), words[i].end(), countX);
        ++i;
        if (words[i] == "C") {
            centerZ = true;
            ++i;
        } else {
            centerZ = false;
        }

        std::from_chars(words[i].begin(), words[i].end(), countZ);
        ++i;

        std::from_chars(words[i].begin(), words[i].end(), spacingX);
        ++i;

        std::from_chars(words[i].begin(), words[i].end(), spacingZ);
        ++i;
    }

    MetaLine Reader::readLine(std::string_view line) {
        uomap_t<std::string_view, std::string_view> parameters;

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
            parameters.insert({key, value});
        }
        if (line.starts_with("SNAP_CLEAR")) {
            return MetaLine(command_variant_t(ClearCommand(parameters)));
        } else if (line.starts_with("SNAP_INCL")) {
            return MetaLine(command_variant_t(InclCommand(parameters)));
        } else if (line.starts_with("SNAP_CYL")) {
            return MetaLine(command_variant_t(CylCommand(parameters)));
        } else if (line.starts_with("SNAP_CLP")) {
            return MetaLine(command_variant_t(ClpCommand(parameters)));
        } else if (line.starts_with("SNAP_FGR")) {
            return MetaLine(command_variant_t(FgrCommand(parameters)));
        } else if (line.starts_with("SNAP_GEN")) {
            return MetaLine(command_variant_t(GenCommand(parameters)));
        } else {
            return MetaLine({});
        }
    }
    ClearCommand::ClearCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
        const auto it = parameters.find("id");
        if (it != parameters.end()) {
            id = std::string(it->second);
        }
    }
    MetaLine::MetaLine(const command_variant_t& data) :
        data(data) {
    }
    InclCommand::InclCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
    CylCommand::CylCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
    ClpCommand::ClpCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
    FgrCommand::FgrCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
    GenCommand::GenCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
}
