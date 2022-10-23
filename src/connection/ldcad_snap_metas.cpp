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

        std::optional<glm::vec3> extractOptionalVec3Parameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                const auto floats = parseStringViewsToFloatArray<3>(stringutil::splitByChar(it->second, ' '));
                return glm::make_vec3(floats.begin());
            } else {
                return {};
            }
        }

        std::optional<glm::mat3> extractOptionalMat3Parameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                const auto floats = parseStringViewsToFloatArray<3 * 3>(stringutil::splitByChar(it->second, ' '));
                return glm::make_mat3(floats.begin());
            } else {
                return {};
            }
        }

        std::vector<float> extractFloatVectorParameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                return parseStringViewsToFloatVector(stringutil::splitByChar(it->second, ' '));
            } else {
                return {};
            }
        }

        std::optional<Grid> extractOptionalGridParameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                return {Grid(it->second)};
            } else {
                return {};
            }
        }

        std::vector<CylShapeBlock> extractCylShapeBlockParameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                const auto words = stringutil::splitByChar(it->second, ' ');
                std::vector<CylShapeBlock> result;
                for (int i = 0; i < words.size(); i += 3) {
                    const auto variant = magic_enum::enum_cast<CylShapeVariant>(words[i]).value();
                    float radius, length;
                    fast_float::from_chars(words[i + 1].begin(), words[i + 1].end(), radius);
                    fast_float::from_chars(words[i + 2].begin(), words[i + 2].end(), length);
                    result.push_back({variant, radius, length});
                }
                return result;
            } else {
                return {};
            }
        }

        bounding_variant_t extractBoundingParameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                const auto words = stringutil::splitByChar(it->second, ' ');
                if (words[0] == "pnt") {
                    return BoundingPnt();
                } else if (words[0] == "box") {
                    BoundingBox box;
                    fast_float::from_chars(words[1].begin(), words[1].end(), box.x);
                    fast_float::from_chars(words[2].begin(), words[2].end(), box.x);
                    fast_float::from_chars(words[3].begin(), words[3].end(), box.x);
                    return box;
                } else if (words[0] == "cube") {
                    BoundingCube cube;
                    fast_float::from_chars(words[1].begin(), words[1].end(), cube.size);
                    return cube;
                } else if (words[0] == "cyl") {
                    BoundingCyl cyl;
                    fast_float::from_chars(words[1].begin(), words[1].end(), cyl.radius);
                    fast_float::from_chars(words[2].begin(), words[2].end(), cyl.length);
                    return cyl;
                } else if (words[0] == "sph") {
                    BoundingSph sph;
                    fast_float::from_chars(words[1].begin(), words[1].end(), sph.radius);
                    return sph;
                } else {
                    throw std::invalid_argument(std::string(words[0]));
                }
            } else {
                throw std::invalid_argument("");
            }
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
    ClearCommand::ClearCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")) {
    }
    MetaLine::MetaLine(command_variant_t data) :
        data(std::move(data)) {
    }
    InclCommand::InclCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")),
        pos(extractOptionalVec3Parameter(parameters, "pos")),
        ori(extractOptionalMat3Parameter(parameters, "ori")),
        scale(extractOptionalVec3Parameter(parameters, "scale")),
        ref(extractOptionalStringParameter(parameters, "mirror").value()),
        grid(extractOptionalGridParameter(parameters, "grid")) {
    }
    CylCommand::CylCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")),
        group(extractOptionalStringParameter(parameters, "group")),
        pos(extractOptionalVec3Parameter(parameters, "pos")),
        ori(extractOptionalMat3Parameter(parameters, "ori")),
        scale(extractEnumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(extractEnumParameter(parameters, "mirror", MirrorType::NONE)),
        gender(extractEnumParameter(parameters, "gender", Gender::M)),
        secs(extractCylShapeBlockParameter(parameters, "secs")),
        caps(extractEnumParameter(parameters, "caps", CylCaps::ONE)),
        grid(extractOptionalGridParameter(parameters, "grid")),
        center(extractBoolParameter(parameters, "center", false)),
        slide(extractBoolParameter(parameters, "slide", false)) {
    }
    ClpCommand::ClpCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")),
        pos(extractOptionalVec3Parameter(parameters, "pos")),
        ori(extractOptionalMat3Parameter(parameters, "ori")),
        radius(extractFloatParameter(parameters, "radius", 4.f)),
        length(extractFloatParameter(parameters, "length", 8.f)),
        center(extractBoolParameter(parameters, "center", false)),
        slide(extractBoolParameter(parameters, "slide", false)),
        scale(extractEnumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(extractEnumParameter(parameters, "mirror", MirrorType::NONE)) {
    }
    FgrCommand::FgrCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")),
        group(extractOptionalStringParameter(parameters, "group")),
        genderOfs(extractEnumParameter(parameters, "genderOfs", Gender::M)),
        radius(extractFloatParameter(parameters, "radius", 0.f)),
        center(extractBoolParameter(parameters, "center", false)),
        scale(extractEnumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(extractEnumParameter(parameters, "mirror", MirrorType ::NONE)),
        pos(extractOptionalVec3Parameter(parameters, "pos")),
        ori(extractOptionalMat3Parameter(parameters, "ori")),
        seq(extractFloatVectorParameter(parameters, "seq")) {
    }
    GenCommand::GenCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")),
        group(extractOptionalStringParameter(parameters, "group")),
        pos(extractOptionalVec3Parameter(parameters, "pos")),
        ori(extractOptionalMat3Parameter(parameters, "ori")),
        bounding(extractBoundingParameter(parameters, "bounding")),
        gender(extractEnumParameter(parameters, "gender", Gender::M)),
        scale(extractEnumParameter(parameters, "scale", ScaleType::NONE)),
        mirror(extractEnumParameter(parameters, "mirror", MirrorType::NONE)) {
    }
}
