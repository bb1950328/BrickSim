// ldr_colors.cpp
// Created by bb1950328 on 06.10.20.
//

#include <fstream>
#include <sstream>
#include <algorithm>
#include "ldr_colors.h"
#include "util.h"
#include "config.h"

LdrColor::LdrColor(const std::string &line) {
    std::stringstream linestream(line);
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
            material = new LdrColorMaterial();
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
    }
}

LdrColor *LdrColorRepository::get_color(const int colorCode) {
    auto iterator = colors.find(colorCode);
    if (iterator == colors.end()) {
        throw std::invalid_argument("unknown color code: " + std::to_string(colorCode));
    }
    return &(iterator->second);
}

LdrColorRepository *LdrColorRepository::instance = nullptr;
LdrInstanceDummyColor LdrColorRepository::instDummyColor;

LdrColorRepository *LdrColorRepository::getInstance() {
    if (instance == nullptr) {
        instance = new LdrColorRepository();
        instance->initialize();
    }
    return instance;
}

void LdrColorRepository::initialize() {
    auto lib_path = util::extend_home_dir(config::get_string(config::LDRAW_PARTS_LIBRARY));
    auto input = std::ifstream(util::pathjoin({lib_path, "LDConfig.ldr"}));
    for (std::string line; getline(input, line);) {
        auto trimmed = util::trim(line);
        if (!trimmed.empty() && trimmed.rfind("0 !COLOUR", 0) == 0) {
            LdrColor col(line.substr(10));
            colors[col.code] = col;
            hueSortedCodes.push_back(col.code);
        }
    }
    colors[instDummyColor.code] = instDummyColor;
    /*for (const auto &colorPair : colors) {
        const auto &color = colorPair.second;
        printf("|%32s|%4d\n", color.name.c_str(), color.alpha);
    }*/
    std::sort(hueSortedCodes.begin(), hueSortedCodes.end(),
              [](const int &a, const int &b){
                  auto repo = LdrColorRepository::getInstance();
                  return util::HSVcolor(repo->get_color(a)->value).hue < util::HSVcolor(repo->get_color(b)->value).hue;
              });
}

std::map<std::string, std::vector<const LdrColor *>> LdrColorRepository::getAllColorsGroupedAndSortedByHue() {
    std::map<std::string, std::vector<const LdrColor *>> result;
    for (const auto &colorPair : colors) {
        if (colorPair.first != LdrColorRepository::instDummyColor.code
            && colorPair.first != 16
            && colorPair.first != 24) {
            result[colorPair.second.getGroupDisplayName()].push_back(&colorPair.second);
        }
    }
    for (const auto &entry : result) {

    }
    return result;
}

LdrInstanceDummyColor::LdrInstanceDummyColor() {
    name = "Instance Dummy Color";
    code = -1;
    value = edge = util::RGBcolor("#FFB39B");
}
