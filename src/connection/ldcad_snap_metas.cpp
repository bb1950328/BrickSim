#include "ldcad_snap_metas.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include "../types.h"
#include "fast_float/fast_float.h"
#include "glm/gtc/type_ptr.hpp"
#include <charconv>
#include <magic_enum.hpp>
#include <utility>
#include <vector>

namespace bricksim::connection::ldcad_snap_meta {
    namespace {
        template<std::size_t N>
        std::array<float, N> parseStringViewsToFloatArray(const std::vector<std::string_view>& strings) {
            assert(strings.size() >= N);
            std::array<float, N> result{};
            for (int i = 0; i < N; ++i) {
                fast_float::from_chars(strings[i].begin(), strings[i].end(), result[i]);
            }
            return result;
        }

        std::vector<float> parseStringViewsToFloatVector(const std::vector<std::string_view>& strings) {
            std::vector<float> result(strings.size());
            for (int i = 0; i < strings.size(); ++i) {
                fast_float::from_chars(strings[i].begin(), strings[i].end(), result[i]);
            }
            return result;
        }

        template<typename E>
        E extractEnumParameter(const parsed_param_container& parameters, const char* const paramName, E defaultValue) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                return magic_enum::enum_cast<E>(it->second, stringutil::charEqualsIgnoreCase).value_or(defaultValue);
            } else {
                return defaultValue;
            }
        }

        std::optional<std::string> extractOptionalStringParameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                return {std::string(it->second)};
            } else {
                return {};
            }
        }

        bool extractBoolParameter(const parsed_param_container& parameters, const char* const paramName, bool defaultValue) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                return stringutil::stringEqualsIgnoreCase(it->second, "true");
            } else {
                return false;
            }
        }

        float extractFloatParameter(const parsed_param_container& parameters, const char* const paramName, float defaultValue) {
            float result = defaultValue;
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                fast_float::from_chars(it->second.begin(), it->second.end(), result);
            }
            return result;
        }
    }
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
    MetaLine::MetaLine(command_variant_t data) :
        data(std::move(data)) {
    }
    InclCommand::InclCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
    CylCommand::CylCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
    ClpCommand::ClpCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
    FgrCommand::FgrCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")),
        group(extractOptionalStringParameter(parameters, "group")),
        genderOfs(extractEnumParameter(parameters, "genderOfs", Gender::M)),
        radius(extractFloatParameter(parameters, "radius", 0.f)),
        center(extractBoolParameter(parameters, "center", false)),
        scale(extractEnumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(extractEnumParameter(parameters, "mirror", MirrorType ::NONE))
    {
        auto it = parameters.find("pos");
        if (it != parameters.end()) {
            const auto floats = parseStringViewsToFloatArray<3>(stringutil::splitByChar(it->second, ' '));
            pos = glm::make_vec3(floats.begin());
        }

        it = parameters.find("ori");
        if (it != parameters.end()) {
            const auto floats = parseStringViewsToFloatArray<3 * 3>(stringutil::splitByChar(it->second, ' '));
            ori = glm::make_mat3(floats.begin());
        }

        it = parameters.find("seq");
        if (it != parameters.end()) {
            seq = parseStringViewsToFloatVector(stringutil::splitByChar(it->second, ' '));
        }
    }
    GenCommand::GenCommand(const uomap_t<std::string_view, std::string_view>& parameters) {
    }
}
