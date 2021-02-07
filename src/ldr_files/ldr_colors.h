// ldr_colors.h
// Created by bb1950328 on 06.10.20.
//

#ifndef BRICKSIM_LDR_COLORS_H
#define BRICKSIM_LDR_COLORS_H

#include <map>
#include <glm/glm.hpp>
#include <vector>
#include "../helpers/util.h"


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

class LdrColorReference;

class LdrColor: public std::enable_shared_from_this<LdrColor> {
public:
    enum Finish {
        NONE, CHROME, PEARLESCENT, RUBBER, MATTE_METALLIC, METAL, MATERIAL
    };

    typedef int code_t;

    [[nodiscard]] std::string getGroupDisplayName() const;
    [[nodiscard]] LdrColorReference asReference() const;

    LdrColor() = default;

    explicit LdrColor(const std::string &line);
    std::string name;
    code_t code;
    util::RGBcolor value;
    util::RGBcolor edge;
    unsigned char alpha = 255;
    unsigned char luminance = 0;
    Finish finish = NONE;
    std::optional<LdrColorMaterial> material{};
    constexpr static int MAIN_COLOR_CODE = 16;
    constexpr static int LINE_COLOR_CODE = 24;
};

class LdrColorReference {
public:
    LdrColorReference();
    LdrColor::code_t code;
    LdrColorReference(LdrColor::code_t code); // NOLINT(google-explicit-constructor)
    explicit LdrColorReference(const std::shared_ptr<LdrColor>& fromColor);
    [[nodiscard]] std::shared_ptr<const LdrColor> get() const;
    bool operator==(const LdrColorReference &rhs) const;
    bool operator!=(const LdrColorReference &rhs) const;
    bool operator<(const LdrColorReference &rhs) const;
    bool operator>(const LdrColorReference &rhs) const;
    bool operator<=(const LdrColorReference &rhs) const;
    bool operator>=(const LdrColorReference &rhs) const;
};

namespace ldr_color_repo {
    constexpr LdrColor::code_t INSTANCE_DUMMY_COLOR_CODE = -1;
    constexpr LdrColor::code_t NO_COLOR_CODE = -2;
    class LdrInstanceDummyColor : public LdrColor {
    public:
        LdrInstanceDummyColor();
    };

    void initialize();
    std::shared_ptr<const LdrColor> get_color(LdrColor::code_t colorCode);
    std::map<std::string, std::vector<LdrColorReference>> getAllColorsGroupedAndSortedByHue();
    std::map<int, std::shared_ptr<LdrColor>> &getColors();
    LdrColorReference getInstanceDummyColor();
};
#endif //BRICKSIM_LDR_COLORS_H
