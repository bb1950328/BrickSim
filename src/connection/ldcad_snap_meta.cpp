#include "ldcad_snap_meta.h"
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
        fast_float::from_chars_result parseFloat(std::string_view sv, float& value) {
            const char* begin = &sv.front();
            const char* end = &sv.back() + 1;
            return fast_float::from_chars(begin, end, value);
        }

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
        std::from_chars_result parseInt(std::string_view sv, T& value) {
            const char* begin = &sv.front();
            const char* end = &sv.back() + 1;
            return std::from_chars(begin, end, value);
        }

        template<std::size_t N>
        std::array<float, N> parseStringViewsToFloatArray(const std::vector<std::string_view>& strings) {
            assert(strings.size() >= N);
            std::array<float, N> result{};
            for (int i = 0; i < N; ++i) {
                parseFloat(strings[i], result[i]);
            }
            return result;
        }

        std::vector<float> parseStringViewsToFloatVector(const std::vector<std::string_view>& strings) {
            std::vector<float> result(strings.size());
            for (int i = 0; i < strings.size(); ++i) {
                parseFloat(strings[i], result[i]);
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
                parseFloat(it->second, result);
            }
            return result;
        }

        std::optional<glm::vec3> extractOptionalVec3Parameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                const auto floats = parseStringViewsToFloatArray<3>(stringutil::splitByChar(it->second, ' '));
                return glm::make_vec3(floats.data());
            } else {
                return {};
            }
        }

        std::optional<glm::mat3> extractOptionalMat3Parameter(const parsed_param_container& parameters, const char* const paramName) {
            const auto it = parameters.find(paramName);
            if (it != parameters.end()) {
                const auto floats = parseStringViewsToFloatArray<3 * 3>(stringutil::splitByChar(it->second, ' '));
                return glm::make_mat3(floats.data());
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
                    CylShapeBlock block{};
                    block.variant = magic_enum::enum_cast<CylShapeVariant>(words[i]).value();
                    parseFloat(words[i + 1], block.radius);
                    parseFloat(words[i + 2], block.length);
                    result.push_back(block);
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
                    BoundingBox box{};
                    parseFloat(words[1], box.x);
                    parseFloat(words[2], box.y);
                    parseFloat(words[3], box.z);
                    return box;
                } else if (words[0] == "cube") {
                    BoundingCube cube{};
                    parseFloat(words[1], cube.size);
                    return cube;
                } else if (words[0] == "cyl") {
                    BoundingCyl cyl{};
                    parseFloat(words[1], cyl.radius);
                    parseFloat(words[2], cyl.length);
                    return cyl;
                } else if (words[0] == "sph") {
                    BoundingSph sph{};
                    parseFloat(words[1], sph.radius);
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

        parseInt(words[i], countX);
        ++i;
        if (words[i] == "C") {
            centerZ = true;
            ++i;
        } else {
            centerZ = false;
        }

        parseInt(words[i], countZ);
        ++i;

        parseFloat(words[i], spacingX);
        ++i;

        parseFloat(words[i], spacingZ);
        ++i;
    }
    bool Grid::operator==(const Grid& rhs) const {
        return centerX == rhs.centerX
               && centerZ == rhs.centerZ
               && countX == rhs.countX
               && countZ == rhs.countZ
               && spacingX == rhs.spacingX
               && spacingZ == rhs.spacingZ;
    }
    bool Grid::operator!=(const Grid& rhs) const {
        return !(rhs == *this);
    }

    std::shared_ptr<MetaLine> Reader::readLine(std::string_view line) {
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
        if (line.starts_with(ClearCommand::NAME)) {
            return std::make_shared<MetaLine>(command_variant_t(ClearCommand(parameters)));
        } else if (line.starts_with(InclCommand::NAME)) {
            return std::make_shared<MetaLine>(command_variant_t(InclCommand(parameters)));
        } else if (line.starts_with(CylCommand::NAME)) {
            return std::make_shared<MetaLine>(command_variant_t(CylCommand(parameters)));
        } else if (line.starts_with(ClpCommand::NAME)) {
            return std::make_shared<MetaLine>(command_variant_t(ClpCommand(parameters)));
        } else if (line.starts_with(FgrCommand::NAME)) {
            return std::make_shared<MetaLine>(command_variant_t(FgrCommand(parameters)));
        } else if (line.starts_with(GenCommand::NAME)) {
            return std::make_shared<MetaLine>(command_variant_t(GenCommand(parameters)));
        } else {
            return std::make_shared<MetaLine>(command_variant_t{});
        }
    }
    ClearCommand::ClearCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")) {
    }
    bool ClearCommand::operator==(const ClearCommand& rhs) const {
        return id == rhs.id;
    }
    bool ClearCommand::operator!=(const ClearCommand& rhs) const {
        return !(rhs == *this);
    }
    MetaLine::MetaLine(command_variant_t data) :
        data(std::move(data)) {
    }
    bool MetaLine::operator==(const MetaLine& rhs) const {
        return data == rhs.data;
    }
    bool MetaLine::operator!=(const MetaLine& rhs) const {
        return !(rhs == *this);
    }
    const char* MetaLine::subcommandName() const {
        if (std::holds_alternative<ClearCommand>(data)) {
            return ClearCommand::NAME;
        } else if (std::holds_alternative<InclCommand>(data)) {
            return InclCommand::NAME;
        } else if (std::holds_alternative<CylCommand>(data)) {
            return CylCommand::NAME;
        } else if (std::holds_alternative<ClpCommand>(data)) {
            return ClpCommand::NAME;
        } else if (std::holds_alternative<FgrCommand>(data)) {
            return FgrCommand::NAME;
        } else if (std::holds_alternative<GenCommand>(data)) {
            return GenCommand::NAME;
        } else {
            throw std::invalid_argument("");
        }
    }
    InclCommand::InclCommand(const uomap_t<std::string_view, std::string_view>& parameters) :
        id(extractOptionalStringParameter(parameters, "id")),
        pos(extractOptionalVec3Parameter(parameters, "pos")),
        ori(extractOptionalMat3Parameter(parameters, "ori")),
        scale(extractOptionalVec3Parameter(parameters, "scale")),
        ref(extractOptionalStringParameter(parameters, "ref").value()),
        grid(extractOptionalGridParameter(parameters, "grid")) {
    }
    bool InclCommand::operator==(const InclCommand& rhs) const {
        return id == rhs.id
               && pos == rhs.pos
               && ori == rhs.ori
               && scale == rhs.scale
               && ref == rhs.ref
               && grid == rhs.grid;
    }
    bool InclCommand::operator!=(const InclCommand& rhs) const {
        return !(rhs == *this);
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
    bool CylCommand::operator!=(const CylCommand& rhs) const {
        return !(rhs == *this);
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
    bool ClpCommand::operator==(const ClpCommand& rhs) const {
        return id == rhs.id
               && pos == rhs.pos
               && ori == rhs.ori
               && radius == rhs.radius
               && length == rhs.length
               && center == rhs.center
               && slide == rhs.slide
               && scale == rhs.scale
               && mirror == rhs.mirror;
    }
    bool ClpCommand::operator!=(const ClpCommand& rhs) const {
        return !(rhs == *this);
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
    bool FgrCommand::operator==(const FgrCommand& rhs) const {
        return id == rhs.id
               && group == rhs.group
               && pos == rhs.pos
               && ori == rhs.ori
               && genderOfs == rhs.genderOfs
               && seq == rhs.seq
               && radius == rhs.radius
               && center == rhs.center
               && scale == rhs.scale
               && mirror == rhs.mirror;
    }
    bool FgrCommand::operator!=(const FgrCommand& rhs) const {
        return !(rhs == *this);
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
    bool GenCommand::operator==(const GenCommand& rhs) const {
        return id == rhs.id
               && group == rhs.group
               && pos == rhs.pos
               && ori == rhs.ori
               && gender == rhs.gender
               && bounding == rhs.bounding
               && scale == rhs.scale
               && mirror == rhs.mirror;
    }
    bool GenCommand::operator!=(const GenCommand& rhs) const {
        return !(rhs == *this);
    }
    bool CylShapeBlock::operator==(const CylShapeBlock& rhs) const {
        return variant == rhs.variant
               && radius == rhs.radius
               && length == rhs.length;
    }
    bool CylShapeBlock::operator!=(const CylShapeBlock& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingPnt::operator==(const BoundingPnt& rhs) const {
        return true;
    }
    bool BoundingPnt::operator!=(const BoundingPnt& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingBox::operator==(const BoundingBox& rhs) const {
        return x == rhs.x
               && y == rhs.y
               && z == rhs.z;
    }
    bool BoundingBox::operator!=(const BoundingBox& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingCube::operator==(const BoundingCube& rhs) const {
        return size == rhs.size;
    }
    bool BoundingCube::operator!=(const BoundingCube& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingCyl::operator==(const BoundingCyl& rhs) const {
        return radius == rhs.radius
               && length == rhs.length;
    }
    bool BoundingCyl::operator!=(const BoundingCyl& rhs) const {
        return !(rhs == *this);
    }
    bool BoundingSph::operator==(const BoundingSph& rhs) const {
        return radius == rhs.radius;
    }
    bool BoundingSph::operator!=(const BoundingSph& rhs) const {
        return !(rhs == *this);
    }
}
