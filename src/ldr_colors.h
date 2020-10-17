// ldr_colors.h
// Created by bb1950328 on 06.10.20.
//

#ifndef BRICKSIM_LDR_COLORS_H
#define BRICKSIM_LDR_COLORS_H

#include <map>
#include <glm/glm.hpp>
#include "util.h"


struct LdrColorMaterial {
    enum Type {
        GLITTER, SPECKLE
    };
    Type type;
    util::RGB value;
    unsigned char alpha;
    unsigned char luminance;
    double fraction, vfraction;
    int size = 0;
    int minsize = 0;
    int maxsize = 0;
};

class LdrColor {
public:
    enum Finish {
        NONE, CHROME, PEARLESCENT, RUBBER, MATTE_METALLIC, METAL, MATERIAL
    };

    LdrColor() = default;

    explicit LdrColor(const std::string &line);

    std::string name;
    int code;
    util::RGB value;
    util::RGB edge;
    unsigned char alpha = 255;
    unsigned char luminance = 0;
    Finish finish = NONE;
    LdrColorMaterial *material = nullptr;
};

class LdrInstanceDummyColor : public LdrColor {
public:
    LdrInstanceDummyColor();
};

class LdrColorRepository {
private:
    static LdrColorRepository *instance;
public:
    std::map<int, LdrColor> colors;
    static LdrColorRepository *getInstance();

public:

    void initialize();

    LdrColor *get_color(int colorCode);

    static LdrInstanceDummyColor instDummyColor;

};
#endif //BRICKSIM_LDR_COLORS_H
