#pragma once

#include "magic_enum.hpp"
#include "base.h"

namespace bricksim::connection::ldcad_meta::write {
    void optionalStringParameter(written_param_container& parameters, const char* paramName, const std::optional<std::string>& value);
    void stringParameter(written_param_container& parameters, const char* paramName, const std::string& value);

    void optionalVec3Parameter(written_param_container& parameters, const char* paramName, const std::optional<glm::vec3>& value);
    void optionalMat3Parameter(written_param_container& parameters, const char* paramName, const std::optional<glm::mat3>& value);

    void optionalGridParameter(written_param_container& parameters, const char* paramName, const std::optional<Grid>& value);
    void cylShapeBlockParameter(written_param_container& parameters, const char* paramName, const std::vector<CylShapeBlock>& value);
    void boundingParameter(written_param_container& parameters, const char* paramName, const bounding_variant_t& value);

    void boolParameter(written_param_container& parameters, const char* paramName, bool value);
    void boolParameter(written_param_container& parameters, const char* paramName, bool value, bool defaultValue);
    void optionalBoolParameter(written_param_container& parameters, const char* paramName, std::optional<bool> value);

    void floatParameter(written_param_container& parameters, const char* paramName, float value);
    void floatParameter(written_param_container& parameters, const char* paramName, float value, float defaultValue);
    void floatVectorParameter(written_param_container& parameters, const char* paramName, const std::vector<float>& value);

    template<typename E>
    void enumParameter(written_param_container& parameters, const char* const paramName, E value) {
        parameters.emplace(paramName, magic_enum::enum_name(value));
    }

    template<typename E>
    void optionalEnumParameter(written_param_container& parameters, const char* const paramName, std::optional<E> value) {
        if (value.has_value()) {
            parameters.emplace(paramName, magic_enum::enum_name(*value));
        }
    }

    template<typename E>
    void enumParameter(written_param_container& parameters, const char* const paramName, E value, E defaultValue) {
        if (value != defaultValue) {
            enumParameter(parameters, paramName, value);
        }
    }
}
