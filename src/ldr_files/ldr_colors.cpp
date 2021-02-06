// ldr_colors.cpp
// Created by bb1950328 on 06.10.20.
//

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include "ldr_colors.h"
#include "../helpers/util.h"
#include "../config.h"
#include "ldr_file_repo.h"


LdrColor::LdrColor(const std::string &line) {
    std::stringstream linestream(line);//todo optimize this one day (using strtok instead of stringstream)
    linestream >> name;
    while (linestream.rdbuf()->in_avail() != 0) {
        std::string keyword;
        linestream >> keyword;
        if (keyword == "CODE") {
            linestream >> code;
        } else if (keyword == "VALUE") {
            std::string valueCode;
            linestream >> valueCode;
            value = util::RGBcolor(valueCode);
        } else if (keyword == "EDGE") {
            std::string edgeCode;
            linestream >> edgeCode;
            edge = util::RGBcolor(edgeCode);
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
            material = {LdrColorMaterial()};
            std::string typeStr;
            linestream >> typeStr;
            material->type = typeStr == "GLITTER" ? LdrColorMaterial::GLITTER : LdrColorMaterial::SPECKLE;
            while (linestream.rdbuf()->in_avail() != 0) {
                linestream >> keyword;
                if (keyword == "VALUE") {
                    std::string valueCode;
                    linestream >> valueCode;
                    material->value = util::RGBcolor(valueCode);
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

std::string LdrColor::getGroupDisplayName() const {
    switch (finish) {
        case NONE:
            if (alpha == 255) {
                return "Solid";
            } else {
                return "Transparent";
            }
        case CHROME:return "Chrome";
        case PEARLESCENT:return "Pearlescent";
        case RUBBER:return "Rubber";
        case MATTE_METALLIC:return "Matte-metallic";
        case METAL:return "Metal";
        case MATERIAL:return "Special Material";
        default:return "Other";
    }
}

namespace ldr_color_repo {
    namespace {
        std::map<int, std::shared_ptr<LdrColor>> colors;
        std::vector<int> hueSortedCodes;
    }

    std::shared_ptr<const LdrColor> get_color(const int colorCode) {
        auto iterator = colors.find(colorCode);
        if (iterator == colors.end()) {
            throw std::invalid_argument("unknown color code: " + std::to_string(colorCode));
        }
        return iterator->second;
    }

    void initialize() {
        static bool initialized = false;
        if (!initialized) {
            std::stringstream inpStream;
            std::string contentString = ldr_file_repo::get().getLibraryFileContent("LDConfig.ldr");
            inpStream << contentString;
            for (std::string line; getline(inpStream, line);) {
                auto trimmed = util::trim(line);
                if (!trimmed.empty() && trimmed.rfind("0 !COLOUR", 0) == 0) {
                    auto col = std::make_shared<LdrColor>(line.substr(10));
                    colors[col->code] = col;
                    hueSortedCodes.push_back(col->code);
                }
            }
            colors[getInstanceDummyColor()->code] = getInstanceDummyColor();
            std::sort(hueSortedCodes.begin(), hueSortedCodes.end(),
                      [](const int &a, const int &b) {
                          return util::HSVcolor(get_color(a)->value).hue < util::HSVcolor(get_color(b)->value).hue;
                      });
            initialized = true;
        }
    }

    std::map<std::string, std::vector<std::shared_ptr<const LdrColor>>> getAllColorsGroupedAndSortedByHue() {
        std::map<std::string, std::vector<std::shared_ptr<const LdrColor>>> result;
        for (const auto &colorPair : colors) {
            if (colorPair.first != getInstanceDummyColor()->code
                && colorPair.first != LdrColor::MAIN_COLOR_CODE
                && colorPair.first != LdrColor::LINE_COLOR_CODE) {
                result[colorPair.second->getGroupDisplayName()].push_back(colorPair.second);
            }
        }
        return result;
    }

    std::map<int, std::shared_ptr<LdrColor>> &getColors() {
        return colors;
    }

    std::shared_ptr<LdrInstanceDummyColor> getInstanceDummyColor() {
        static auto instDummyColor = std::make_shared<LdrInstanceDummyColor>();
        return instDummyColor;
    }

    LdrInstanceDummyColor::LdrInstanceDummyColor() {
        name = "Instance Dummy Color";
        code = -1;
        value = edge = util::RGBcolor("#FFB39B");
    }
}