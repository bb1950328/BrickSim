// ldr_colors.cpp
// Created by bab21 on 06.10.20.
//

#include <fstream>
#include <sstream>
#include "ldr_colors.h"
#include "util.h"
#include "config.h"

LdrColor::LdrColor(const std::string& line) {
    std::stringstream linestream(line);
    linestream >> name;
    while (linestream.rdbuf()->in_avail() != 0) {
        std::string keyword;
        linestream >> keyword;
        if (keyword=="CODE") {
            linestream>>code;
        } else if (keyword=="VALUE") {
            std::string valueCode;
            linestream>>valueCode;
            value = RGB(valueCode);
        } else if (keyword=="EDGE") {
            std::string edgeCode;
            linestream>>edgeCode;
            edge = RGB(edgeCode);
        } else if (keyword=="CHROME") {
            finish = CHROME;
        } else if (keyword=="PEARLESCENT") {
            finish = PEARLESCENT;
        } else if (keyword=="RUBBER") {
            finish = RUBBER;
        } else if (keyword=="MATTE_METALLIC") {
            finish = MATTE_METALLIC;
        } else if (keyword=="METAL") {
            finish = METAL;
        } else if (keyword=="MATERIAL") {
            finish = MATERIAL;
            material = new LdrColorMaterial();
            std::string typeStr;
            linestream>>typeStr;
            material->type = typeStr=="GLITTER"?LdrColorMaterial::GLITTER:LdrColorMaterial::SPECKLE;
            while (linestream.rdbuf()->in_avail() != 0) {
                linestream>>keyword;
                if (keyword=="VALUE") {
                    std::string valueCode;
                    linestream>>valueCode;
                    material->value = RGB(valueCode);
                } else if (keyword=="ALPHA") {
                    linestream>>material->alpha;
                } else if (keyword=="LUMINANCE") {
                    linestream>>material->luminance;
                } else if (keyword=="FRACTION") {
                    linestream>>material->fraction;
                } else if (keyword=="VFRACTION") {
                    linestream>>material->vfraction;
                } else if (keyword=="SIZE") {
                    linestream>>material->size;
                } else if (keyword=="MAXSIZE") {
                    linestream>>material->maxsize;
                } else if (keyword=="MINSIZE") {
                    linestream>>material->minsize;
                }
            }
            break;
        }
    }
}
glm::vec3 RGB::asGlmVector() const {
    return glm::vec3(red/255.0f, green/255.0f, blue/255.0f);
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
LdrColorRepository * LdrColorRepository::getInstance(){
    if (instance == nullptr) {
        instance = new LdrColorRepository();
        instance->initialize();
    }
    return instance;
}
void LdrColorRepository::initialize(){
    auto lib_path = util::extend_home_dir(Configuration::getInstance()->get_string(config::KEY_LDRAW_PARTS_LIBRARY));
    auto input = std::ifstream(util::pathjoin({lib_path, "LDConfig.ldr"}));
    for (std::string line; getline(input, line);) {
        auto trimmed = util::trim(line);
        if (!trimmed.empty() && trimmed.rfind("0 !COLOUR", 0)==0) {
            LdrColor col(line.substr(10));
            colors[col.code] = col;
        }
    }
    colors[instDummyColor.code] = instDummyColor;
}

RGB::RGB(std::string htmlCode){
    auto redChars = new char[2];
    auto greenChars = new char[2];
    auto blueChars = new char[2];

    redChars[0] = htmlCode[1];
    redChars[1] = htmlCode[2];
    greenChars[0] = htmlCode[3];
    greenChars[1] = htmlCode[4];
    blueChars[0] = htmlCode[5];
    blueChars[1] = htmlCode[6];

    red = std::stoi(redChars,nullptr,16);
    green = std::stoi(greenChars,nullptr,16);
    blue = std::stoi(blueChars,nullptr,16);

    delete [] redChars;
    delete [] greenChars;
    delete [] blueChars;
}
LdrInstanceDummyColor::LdrInstanceDummyColor() {
    name = "Instance Dummy Color";
    code = -1;
    value = edge = RGB("#FFB39B");
}
