#include "colors.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include "config.h"
#include "file_repo.h"
#include <sstream>

namespace bricksim::ldr {
    Color::Color(const std::string_view line) {
        std::stringstream linestream((std::string(line)));//todo optimize this one day (using strtok instead of stringstream)
        linestream >> name;
        while (linestream.rdbuf()->in_avail() != 0) {
            std::string keyword;
            linestream >> keyword;
            if (keyword == "CODE") {
                linestream >> code;
            } else if (keyword == "VALUE") {
                std::string valueCode;
                linestream >> valueCode;
                value = color::RGB(valueCode);
            } else if (keyword == "EDGE") {
                std::string edgeCode;
                linestream >> edgeCode;
                edge = color::RGB(edgeCode);
            } else if (keyword == "ALPHA") {
                linestream >> alpha;
            } else if (keyword == "CHROME") {
                finish = CHROME;
            } else if (keyword == "PEARLESCENT") {
                finish = PEARLESCENT;
            } else if (keyword == "RUBBER") {
                finish = RUBBER;
            } else if (keyword == "MATTE_METALLIC") {
                finish = MATTE_METALLIC;
            } else if (keyword == "METAL") {
                finish = METAL;
            } else if (keyword == "MATERIAL") {
                finish = MATERIAL;
                material = {ColorMaterial()};
                std::string typeStr;
                linestream >> typeStr;
                material->type = typeStr == "GLITTER" ? ColorMaterial::GLITTER : ColorMaterial::SPECKLE;
                while (linestream.rdbuf()->in_avail() != 0) {
                    linestream >> keyword;
                    if (keyword == "VALUE") {
                        std::string valueCode;
                        linestream >> valueCode;
                        material->value = color::RGB(valueCode);
                    } else if (keyword == "ALPHA") {
                        linestream >> material->alpha;
                    } else if (keyword == "LUMINANCE") {
                        linestream >> material->luminance;
                    } else if (keyword == "FRACTION") {
                        linestream >> material->fraction;
                    } else if (keyword == "VFRACTION") {
                        linestream >> material->vfraction;
                    } else if (keyword == "SIZE") {
                        linestream >> material->size;
                    } else if (keyword == "MAXSIZE") {
                        linestream >> material->maxsize;
                    } else if (keyword == "MINSIZE") {
                        linestream >> material->minsize;
                    }
                }
                break;
            }
        }
    }

    std::string Color::getGroupDisplayName() const {
        switch (finish) {
            case NONE:
                if (alpha == 255) {
                    return "Solid";
                } else {
                    return "Transparent";
                }
            case CHROME:
                return "Chrome";
            case PEARLESCENT:
                return "Pearlescent";
            case RUBBER:
                return "Rubber";
            case MATTE_METALLIC:
                return "Matte-metallic";
            case METAL:
                return "Metal";
            case MATERIAL:
                return "Special Material";
            default:
                return "Other";
        }
    }

    ColorReference Color::asReference() const {
        return ColorReference(code);
    }

    namespace color_repo {
        namespace {
            uomap_t<Color::code_t, std::shared_ptr<Color>> colors;
            uomap_t<std::string, ColorReference> pureColors;
            std::vector<std::pair<float, Color::code_t>> hueSortedCodes;
            std::shared_ptr<LdrInstanceDummyColor> instDummyColor;

            Color::code_t getUnusedCode() {
                static int lastUsedCode = INSTANCE_DUMMY_COLOR_CODE;
                return --lastUsedCode;
            }
        }

        std::shared_ptr<const Color> getColor(int colorCode) {
            auto iterator = colors.find(colorCode);
            if (iterator == colors.end()) {
                return colors[1];
                //todo make this configurable
                // throw std::invalid_argument("unknown color code: " + std::to_string(colorCode));
            }
            return iterator->second;
        }

        void initialize() {
            static bool initialized = false;
            if (!initialized) {
                for (const auto& col: getConfig().getColors()) {
                    colors[col->code] = col;
                    hueSortedCodes.push_back({color::HSV(col->value).hue, col->code});
                }
                instDummyColor = std::make_shared<LdrInstanceDummyColor>();
                colors[INSTANCE_DUMMY_COLOR_CODE] = instDummyColor;
                std::sort(hueSortedCodes.begin(), hueSortedCodes.end(), util::compare_pair_first<float, Color::code_t>());
                initialized = true;
            }
        }

        omap_t<std::string, std::vector<ColorReference>> getAllColorsGroupedAndSortedByHue() {
            omap_t<std::string, std::vector<ColorReference>> result;
            for (const auto& colorPair: colors) {
                if (colorPair.second->visibleInLists
                    && colorPair.first != Color::MAIN_COLOR_CODE
                    && colorPair.first != Color::LINE_COLOR_CODE) {
                    result[colorPair.second->getGroupDisplayName()].push_back(colorPair.second->asReference());
                }
            }
            return result;
        }

        uomap_t<int, std::shared_ptr<Color>>& getColors() {
            return colors;
        }

        ColorReference getInstanceDummyColor() {
            return ColorReference(INSTANCE_DUMMY_COLOR_CODE);
        }

        ColorReference getPureColor(const std::string& htmlCode) {
            auto it = pureColors.find(htmlCode);
            if (it == pureColors.end()) {
                auto color = std::make_shared<PureColor>(htmlCode);
                colors[color->code] = color;
                return pureColors[htmlCode] = color->code;
            }
            return it->second;
        }

        ColorReference getPureColor(const color::RGB& color) {
            return getPureColor(color.asHtmlCode());
        }

        LdrInstanceDummyColor::LdrInstanceDummyColor() {
            name = "Instance Dummy Color";
            code = INSTANCE_DUMMY_COLOR_CODE;
            value = edge = color::RGB("#FFB39B");
            visibleInLists = false;
        }

        PureColor::PureColor(const std::string& hexCode) {
            name = "Pure " + hexCode;
            code = getUnusedCode();
            value = edge = color::RGB(hexCode);
            finish = PURE;
            visibleInLists = false;
        }

        PureColor::PureColor(color::RGB color) {
            name = "Pure " + color.asHtmlCode();
            code = getUnusedCode();
            value = edge = color;
            finish = PURE;
            visibleInLists = false;
        }
    }

    std::shared_ptr<const Color> ColorReference::get() const {
        return color_repo::getColor(code);
    }

    bool ColorReference::operator==(const ColorReference& rhs) const {
        return code == rhs.code;
    }

    bool ColorReference::operator!=(const ColorReference& rhs) const {
        return !(rhs == *this);
    }

    bool ColorReference::operator<(const ColorReference& rhs) const {
        return code < rhs.code;
    }

    bool ColorReference::operator>(const ColorReference& rhs) const {
        return rhs < *this;
    }

    bool ColorReference::operator<=(const ColorReference& rhs) const {
        return !(rhs < *this);
    }

    bool ColorReference::operator>=(const ColorReference& rhs) const {
        return !(*this < rhs);
    }

    ColorReference::ColorReference(int code) :
        code(code) {}

    ColorReference::ColorReference() :
        code(color_repo::NO_COLOR_CODE) {}

    ColorReference::ColorReference(const std::shared_ptr<Color>& fromColor) :
        code(fromColor->code) {}
}
