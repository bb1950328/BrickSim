#pragma once

#include "../helpers/color.h"
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace bricksim::ldr {
    struct ColorMaterial {
        enum Type {
            GLITTER,
            SPECKLE
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
            NONE,
            CHROME,
            PEARLESCENT,
            RUBBER,
            MATTE_METALLIC,
            METAL,
            MATERIAL,
            PURE
        };

        typedef int code_t;

        [[nodiscard]] std::string getGroupDisplayName() const;
        [[nodiscard]] ColorReference asReference() const;

        Color() = default;

        explicit Color(const std::string& line);
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
        ColorReference(Color::code_t code);// NOLINT(google-explicit-constructor)
        explicit ColorReference(const std::shared_ptr<Color>& fromColor);
        [[nodiscard]] std::shared_ptr<const Color> get() const;
        bool operator==(const ColorReference& rhs) const;
        bool operator!=(const ColorReference& rhs) const;
        bool operator<(const ColorReference& rhs) const;
        bool operator>(const ColorReference& rhs) const;
        bool operator<=(const ColorReference& rhs) const;
        bool operator>=(const ColorReference& rhs) const;
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
            explicit PureColor(const std::string& hexCode);
            explicit PureColor(color::RGB color);
        };

        void initialize();
        std::shared_ptr<const Color> getColor(Color::code_t colorCode);
        omap_t<std::string, std::vector<ColorReference>> getAllColorsGroupedAndSortedByHue();
        uomap_t<int, std::shared_ptr<Color>>& getColors();
        ColorReference getInstanceDummyColor();
        ColorReference getPureColor(const std::string& htmlCode);
        ColorReference getPureColor(const color::RGB& color);
    }
}

namespace std {
    template<>
    struct hash<bricksim::ldr::ColorReference> {
        std::size_t operator()(bricksim::ldr::ColorReference value) const {
            return hash<int>()(value.code);
        }
    };
}
