#ifndef BRICKSIM_LDR_COLORS_H
#define BRICKSIM_LDR_COLORS_H

#include "../helpers/color.h"
#include <memory>
#include <map>
#include <vector>
#include <optional>

namespace bricksim::ldr {
    struct ColorMaterial {
        enum Type {
            GLITTER, SPECKLE
        };
        Type type;
        color::RGB value;
        unsigned char alpha;
        unsigned char luminance;
        double fraction, vfraction;
        int size = 0;
        int minsize = 0;
        int maxsize = 0;
    };

    class ColorReference;

    class Color : public std::enable_shared_from_this<Color> {
    public:
        enum Finish {
            NONE, CHROME, PEARLESCENT, RUBBER, MATTE_METALLIC, METAL, MATERIAL
        };

        typedef int code_t;

        [[nodiscard]] std::string getGroupDisplayName() const;
        [[nodiscard]] ColorReference asReference() const;

        Color() = default;

        explicit Color(const std::string &line);
        std::string name;
        code_t code;
        color::RGB value;
        color::RGB edge;
        unsigned char alpha = 255;
        unsigned char luminance = 0;
        Finish finish = NONE;
        std::optional<ColorMaterial> material{};
        bool visibleInLists = true;

        constexpr static int MAIN_COLOR_CODE = 16;
        constexpr static int LINE_COLOR_CODE = 24;
    };

    class ColorReference {
    public:
        ColorReference();
        Color::code_t code;
        ColorReference(Color::code_t code); // NOLINT(google-explicit-constructor)
        explicit ColorReference(const std::shared_ptr<Color> &fromColor);
        [[nodiscard]] std::shared_ptr<const Color> get() const;
        bool operator==(const ColorReference &rhs) const;
        bool operator!=(const ColorReference &rhs) const;
        bool operator<(const ColorReference &rhs) const;
        bool operator>(const ColorReference &rhs) const;
        bool operator<=(const ColorReference &rhs) const;
        bool operator>=(const ColorReference &rhs) const;
    };

    namespace color_repo {
        constexpr Color::code_t INSTANCE_DUMMY_COLOR_CODE = -1;
        constexpr Color::code_t NO_COLOR_CODE = -2;

        class LdrInstanceDummyColor : public Color {
        public:
            LdrInstanceDummyColor();
        };

        class PureColor : public Color {
        public:
            PureColor(const char *hexCode);
        };

        void initialize();
        std::shared_ptr<const Color> get_color(Color::code_t colorCode);
        std::map<std::string, std::vector<ColorReference>> getAllColorsGroupedAndSortedByHue();
        std::map<int, std::shared_ptr<Color>> &getColors();
        ColorReference getInstanceDummyColor();
        ColorReference getPureColor(const char *htmlCode);
    };
}
#endif //BRICKSIM_LDR_COLORS_H
