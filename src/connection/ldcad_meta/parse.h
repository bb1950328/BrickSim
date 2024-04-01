#pragma once
#include "fast_float/fast_float.h"
#include "magic_enum.hpp"
#include "base.h"
#include <array>
#include <optional>
#include <vector>
#include <charconv>

namespace bricksim::connection::ldcad_meta::parse {
    fast_float::from_chars_result floatFromString(std::string_view sv, float& value);

    template<typename T>
    concept anyIntType = std::is_same_v<int8_t, T>
                         || std::is_same_v<int16_t, T>
                         || std::is_same_v<int32_t, T>
                         || std::is_same_v<int64_t, T>
                         || std::is_same_v<uint8_t, T>
                         || std::is_same_v<uint16_t, T>
                         || std::is_same_v<uint32_t, T>
                         || std::is_same_v<uint64_t, T>;

    template<anyIntType T>
    std::from_chars_result intFromString(std::string_view sv, T &value) {
        const char *begin = &sv.front();
        const char *end = &sv.back() + 1;
        return std::from_chars(begin, end, value);
    }

    template<std::size_t N>
    std::array<float, N> stringViewsToFloatArray(const std::vector<std::string_view>& strings) {
        assert(strings.size() >= N);
        std::array<float, N> result{};
        for (size_t i = 0; i < N; ++i) {
            floatFromString(strings[i], result[i]);
        }
        return result;
    }

    std::vector<float> stringViewsToFloatVector(const std::vector<std::string_view>& strings);

    template<typename E>
    E enumParameter(const parsed_param_container& parameters, const char* const paramName, E defaultValue) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            return magic_enum::enum_cast<E>(it->second, stringutil::charEqualsIgnoreCase).value_or(defaultValue);
        } else {
            return defaultValue;
        }
    }

    template<typename E>
    std::optional<E> optionalEnumParameter(const parsed_param_container& parameters, const char* const paramName) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            return magic_enum::enum_cast<E>(it->second, stringutil::charEqualsIgnoreCase);
        } else {
            return {};
        }
    }

    std::optional<std::string> optionalStringParameter(const parsed_param_container& parameters, const char* paramName);
    bool boolParameter(const parsed_param_container& parameters, const char* paramName, bool defaultValue);
    std::optional<bool> optionalBoolParameter(const parsed_param_container& parameters, const char* paramName);
    float floatParameter(const parsed_param_container& parameters, const char* paramName, float defaultValue);
    std::vector<float> floatVectorParameter(const parsed_param_container& parameters, const char* paramName);
    std::optional<glm::vec3> optionalVec3Parameter(const parsed_param_container& parameters, const char* paramName);
    std::optional<glm::mat3> optionalMat3Parameter(const parsed_param_container& parameters, const char* paramName);
    std::optional<Grid> optionalGridParameter(const parsed_param_container& parameters, const char* paramName);
    std::vector<CylShapeBlock> cylShapeBlockParameter(const parsed_param_container& parameters, const char* paramName);
    bounding_variant_t boundingParameter(const parsed_param_container& parameters, const char* paramName);
}
