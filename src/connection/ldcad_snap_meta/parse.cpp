#include "parse.h"
#include "glm/gtc/type_ptr.hpp"
namespace bricksim::connection::ldcad_snap_meta::parse {
    fast_float::from_chars_result floatFromString(std::string_view sv, float& value) {
        const char* begin = &sv.front();
        const char* end = &sv.back() + 1;
        return fast_float::from_chars(begin, end, value);
    }
    std::vector<float> stringViewsToFloatVector(const std::vector<std::string_view>& strings) {
        std::vector<float> result(strings.size());
        for (int i = 0; i < strings.size(); ++i) {
            floatFromString(strings[i], result[i]);
        }
        return result;
    }
    std::optional<std::string> optionalStringParameter(const parsed_param_container& parameters, const char* paramName) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            return {std::string(it->second)};
        } else {
            return {};
        }
    }
    bool boolParameter(const parsed_param_container& parameters, const char* paramName, bool defaultValue) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            return stringutil::stringEqualsIgnoreCase(it->second, "true");
        } else {
            return defaultValue;
        }
    }
    float floatParameter(const parsed_param_container& parameters, const char* paramName, float defaultValue) {
        float result = defaultValue;
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            floatFromString(it->second, result);
        }
        return result;
    }
    std::optional<glm::vec3> optionalVec3Parameter(const parsed_param_container& parameters, const char* paramName) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            const auto floats = stringViewsToFloatArray<3>(stringutil::splitByChar(it->second, ' '));
            return glm::make_vec3(floats.data());
        } else {
            return {};
        }
    }
    std::optional<glm::mat3> optionalMat3Parameter(const parsed_param_container& parameters, const char* paramName) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            const auto floats = stringViewsToFloatArray<3 * 3>(stringutil::splitByChar(it->second, ' '));
            return glm::transpose(glm::make_mat3(floats.data()));
        } else {
            return {};
        }
    }
    std::vector<float> floatVectorParameter(const parsed_param_container& parameters, const char* paramName) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            return stringViewsToFloatVector(stringutil::splitByChar(it->second, ' '));
        } else {
            return {};
        }
    }
    std::optional<Grid> optionalGridParameter(const parsed_param_container& parameters, const char* paramName) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            return {Grid(it->second)};
        } else {
            return {};
        }
    }
    std::vector<CylShapeBlock> cylShapeBlockParameter(const parsed_param_container& parameters, const char* paramName) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            const auto words = stringutil::splitByChar(it->second, ' ');
            std::vector<CylShapeBlock> result;
            for (int i = 0; i < words.size(); i += 3) {
                CylShapeBlock block{};
                block.variant = magic_enum::enum_cast<CylShapeVariant>(words[i]).value();
                floatFromString(words[i + 1], block.radius);
                floatFromString(words[i + 2], block.length);
                result.push_back(block);
            }
            return result;
        } else {
            return {};
        }
    }
    bounding_variant_t boundingParameter(const parsed_param_container& parameters, const char* paramName) {
        const auto it = parameters.find(paramName);
        if (it != parameters.end()) {
            const auto words = stringutil::splitByChar(it->second, ' ');
            if (words[0] == "pnt") {
                return BoundingPnt();
            } else if (words[0] == "box") {
                BoundingBox box{};
                floatFromString(words[1], box.x);
                floatFromString(words[2], box.y);
                floatFromString(words[3], box.z);
                return box;
            } else if (words[0] == "cube") {
                BoundingCube cube{};
                floatFromString(words[1], cube.size);
                return cube;
            } else if (words[0] == "cyl") {
                BoundingCyl cyl{};
                floatFromString(words[1], cyl.radius);
                floatFromString(words[2], cyl.length);
                return cyl;
            } else if (words[0] == "sph") {
                BoundingSph sph{};
                floatFromString(words[1], sph.radius);
                return sph;
            } else {
                throw std::invalid_argument(std::string(words[0]));
            }
        } else {
            throw std::invalid_argument("");
        }
    }

}
