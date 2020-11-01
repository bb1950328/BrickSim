// ldr_colors.h
// Created by bb1950328 on 06.10.20.
//

#ifndef BRICKSIM_LDR_COLORS_H
#define BRICKSIM_LDR_COLORS_H

#include <map>
#include <glm/glm.hpp>
#include <vector>
#include "helpers/util.h"


struct LdrColorMaterial {
    enum Type {
        GLITTER, SPECKLE
    };
    Type type;
    util::RGBcolor value;
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

    std::string getGroupDisplayName() const;

    LdrColor() = default;

    explicit LdrColor(const std::string &line);

    std::string name;
    int code;
    util::RGBcolor value;
    util::RGBcolor edge;
    unsigned char alpha = 255;
    unsigned char luminance = 0;
    Finish finish = NONE;
    LdrColorMaterial *material = nullptr;
    const static int MAIN_COLOR_CODE = 16;
    const static int LINE_COLOR_CODE = 24;
};



namespace ldr_color_repo {
    class LdrInstanceDummyColor : public LdrColor {
    public:
        LdrInstanceDummyColor();
    };
    void initialize();

    LdrColor *get_color(int colorCode);
    std::map<std::string, std::vector<const LdrColor *>> getAllColorsGroupedAndSortedByHue();
    std::map<int, LdrColor> &getColors();
    LdrInstanceDummyColor& getInstanceDummyColor();
};
#endif //BRICKSIM_LDR_COLORS_H
