#include "write.h"
#include <spdlog/fmt/fmt.h>

namespace bricksim::connection::ldcad_meta::write {
    void optionalStringParameter(written_param_container& parameters, const char* paramName, const std::optional<std::string>& value) {
        if (value.has_value()) {
            parameters.insert({paramName, value.value()});
        }
    }
    void optionalVec3Parameter(written_param_container& parameters, const char* paramName, const std::optional<glm::vec3>& value) {
        if (value.has_value()) {
            const auto& vec = *value;
            parameters.insert({paramName, fmt::format("{:g} {:g} {:g}", vec.x, vec.y, vec.z)});
        }
    }
    void optionalMat3Parameter(written_param_container& parameters, const char* paramName, const std::optional<glm::mat3>& value) {
        if (value.has_value()) {
            const auto& m = *value;
            parameters.insert({paramName, fmt::format("{:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g} {:g}",
                                                      m[0][0], m[0][1], m[0][2],
                                                      m[1][0], m[1][1], m[1][2],
                                                      m[2][0], m[2][1], m[2][2])});
        }
    }
    void stringParameter(written_param_container& parameters, const char* paramName, const std::string& value) {
        parameters.insert({paramName, value});
    }
    void optionalGridParameter(written_param_container& parameters, const char* paramName, const std::optional<Grid>& value) {
        if (value.has_value()) {
            const auto& grid = *value;
            std::string result;
            if (grid.centerX) {
                result.push_back('C');
                result.push_back(' ');
            }
            result.append(fmt::format("{} ", grid.countX));

            if (grid.centerZ) {
                result.push_back('C');
                result.push_back(' ');
            }
            result.append(fmt::format("{} {:g} {:g}", grid.countZ, grid.spacingX, grid.spacingZ));

            parameters.insert({paramName, result});
        }
    }
    void cylShapeBlockParameter(written_param_container& parameters, const char* paramName, const std::vector<CylShapeBlock>& value) {
        if (!value.empty()) {
            std::string result;
            for (const auto& item: value) {
                result.append(fmt::format("{:s} {:g} {:g}  ", magic_enum::enum_name(item.variant), item.radius, item.length));
            }
            if (!result.empty()) {
                result.pop_back();
                result.pop_back();
            }
            parameters.insert({paramName, result});
        }
    }
    void boolParameter(written_param_container& parameters, const char* paramName, bool value) {
        parameters.insert({paramName, value ? "true" : "false"});
    }
    void boolParameter(written_param_container& parameters, const char* paramName, bool value, bool defaultValue) {
        if (value != defaultValue) {
            boolParameter(parameters, paramName, value);
        }
    }
    void optionalBoolParameter(written_param_container& parameters, const char* paramName, std::optional<bool> value) {
        if (value.has_value()) {
            boolParameter(parameters, paramName, *value);
        }
    }

    void floatParameter(written_param_container& parameters, const char* const paramName, float value) {
        parameters.insert({paramName, fmt::format("{:g}", value)});
    }
    void floatParameter(written_param_container& parameters, const char* const paramName, float value, float defaultValue) {
        if (std::fabs(value - defaultValue) > .0001f) {
            parameters.insert({paramName, fmt::format("{:g}", value)});
        }
    }
    void floatVectorParameter(written_param_container& parameters, const char* const paramName, const std::vector<float>& value) {
        std::string result;
        for (const auto& item: value) {
            result.append(fmt::format("{:g} ", item));
        }
        if (!result.empty()) {
            result.pop_back();
        }
        parameters.insert({paramName, result});
    }
    std::string boundingToString(const bounding_variant_t& bounding) {
        if (std::holds_alternative<BoundingPnt>(bounding)) {
            return "pnt";
        } else if (std::holds_alternative<BoundingBox>(bounding)) {
            const auto& box = std::get<BoundingBox>(bounding);
            return fmt::format("box {:g} {:g} {:g}", box.radius.x, box.radius.y, box.radius.z);
        } else if (std::holds_alternative<BoundingCube>(bounding)) {
            const auto& cube = std::get<BoundingCube>(bounding);
            return fmt::format("cube {:g}", cube.radius);
        } else if (std::holds_alternative<BoundingCyl>(bounding)) {
            const auto& cyl = std::get<BoundingCyl>(bounding);
            return fmt::format("cyl {:g} {:g}", cyl.radius, cyl.length);
        } else if (std::holds_alternative<BoundingSph>(bounding)) {
            const auto& sph = std::get<BoundingSph>(bounding);
            return fmt::format("sph {:g}", sph.radius);
        } else {
            throw std::invalid_argument("");
        }
    }
    void boundingParameter(written_param_container& parameters, const char* paramName, const bounding_variant_t& value) {
        parameters.insert({paramName, boundingToString(value)});
    }
}
