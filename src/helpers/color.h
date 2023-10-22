#pragma once

#include "../types.h"
#include <glm/glm.hpp>
#include <random>
#include <string>

//RGB is a macro in some windows header
#ifdef RGB
    #undef RGB
#endif

namespace bricksim::color {
    glm::vec3 convertIntToColorVec3(unsigned int value);
    unsigned int getIntFromColor(unsigned char red, unsigned char green, unsigned char blue);

    class HSV;

    class RGB {
    public:
        constexpr RGB() :
            red(0), green(0), blue(0) {}
        constexpr ~RGB() = default;

        explicit RGB(const std::string& htmlCode);
        explicit RGB(glm::vec3 vector);
        explicit RGB(const HSV& hsv);

        constexpr RGB(color_component_t red, color_component_t green, color_component_t blue) :
            red(red), green(green), blue(blue) {
        }

        color_component_t red;
        color_component_t green;
        color_component_t blue;
        [[nodiscard]] glm::vec3 asGlmVector() const;
        [[nodiscard]] std::string asHtmlCode() const;

        template<typename T>
            requires std::is_arithmetic_v<T>
        RGB operator*(T factor) const {
            return RGB(clampResult(static_cast<T>(red * factor)), clampResult(static_cast<T>(green * factor)), clampResult(static_cast<T>(blue * factor)));
        }

        template<typename T>
            requires std::is_arithmetic_v<T>
        RGB operator+(T value) const {
            return RGB(clampResult(static_cast<T>(red + value)), clampResult(static_cast<T>(green + value)), clampResult(static_cast<T>(blue + value)));
        }

        template<typename T>
            requires std::is_arithmetic_v<T>
        RGB operator-(T value) const {
            return RGB(clampResult(static_cast<T>(red - value)), clampResult(static_cast<T>(green - value)), clampResult(static_cast<T>(blue - value)));
        }

        template<typename T>
            requires std::is_arithmetic_v<T>
        RGB operator/(T value) const {
            return RGB(clampResult(static_cast<T>(red / value)), clampResult(static_cast<T>(green) / value), clampResult(static_cast<T>(blue / value)));
        }

        template<typename T>
            requires std::is_arithmetic_v<T>
        RGB& operator*=(T value) {
            red = clampResult(static_cast<T>(red * value));
            green = clampResult(static_cast<T>(green * value));
            blue = clampResult(static_cast<T>(blue * value));
            return *this;
        }
        template<typename T>
            requires std::is_arithmetic_v<T>
        RGB& operator+=(T value) {
            red = clampResult(static_cast<T>(red + value));
            green = clampResult(static_cast<T>(green + value));
            blue = clampResult(static_cast<T>(blue + value));
            return *this;
        }
        template<typename T>
            requires std::is_arithmetic_v<T>
        RGB& operator/=(T value) {
            red = clampResult(static_cast<T>(red / value));
            green = clampResult(static_cast<T>(green / value));
            blue = clampResult(static_cast<T>(blue / value));
            return *this;
        }
        template<typename T>
            requires std::is_arithmetic_v<T>
        RGB& operator-=(T value) {
            red = clampResult(static_cast<T>(red - value));
            green = clampResult(static_cast<T>(green - value));
            blue = clampResult(static_cast<T>(blue - value));
            return *this;
        }

        RGB operator*(const RGB& other) const {
            return {clampResult(this->red * other.red), clampResult(this->green * other.green), clampResult(this->blue * other.blue)};
        }

        RGB operator+(const RGB& other) const {
            return {clampResult(this->red + other.red), clampResult(this->green + other.green), clampResult(this->blue + other.blue)};
        }

        RGB operator-(const RGB& other) const {
            return {clampResult(this->red - other.red), clampResult(this->green - other.green), clampResult(this->blue - other.blue)};
        }

        RGB operator/(const RGB& other) const {
            return {clampResult(this->red / other.red), clampResult(this->green / other.green), clampResult(this->blue / other.blue)};
        }

        bool operator==(const RGB& rhs) const;
        bool operator!=(const RGB& rhs) const;
        bool operator<(const RGB& rhs) const;
        bool operator>(const RGB& rhs) const;
        bool operator<=(const RGB& rhs) const;
        bool operator>=(const RGB& rhs) const;

    private:
        template<typename T>
            requires std::is_arithmetic_v<T>
        static color_component_t clampResult(T result) {
            return std::clamp(result, static_cast<T>(0), static_cast<T>(255));
        }

        void assignFloat(float r, float g, float b);
    };

    class HSV {
    public:
        HSV() = default;
        explicit HSV(glm::vec3 vector);
        explicit HSV(RGB rgb);
        HSV(color_component_t hue, color_component_t saturation, color_component_t value);

        color_component_t hue;
        color_component_t saturation;
        color_component_t value;
        [[nodiscard]] glm::vec3 asGlmVector() const;

        bool operator==(const HSV& rhs) const;
        bool operator!=(const HSV& rhs) const;
        bool operator<(const HSV& rhs) const;
        bool operator>(const HSV& rhs) const;
        bool operator<=(const HSV& rhs) const;
        bool operator>=(const HSV& rhs) const;
    };

    /**
     * the RNG is initialized with a static seed to get reproducible results
     * @return a random color. the saturation and the value of the HSV representation are always 255.
     */
    RGB getRandom();

    constexpr RGB BLACK = {0, 0, 0};
    constexpr RGB WHITE = {255, 255, 255};
    constexpr RGB RED = {255, 0, 0};
    constexpr RGB LIME = {0, 255, 0};
    constexpr RGB BLUE = {0, 0, 255};
    constexpr RGB YELLOW = {255, 255, 0};
    constexpr RGB CYAN = {0, 255, 255};
    constexpr RGB MAGENTA = {255, 0, 255};
    constexpr RGB SILVER = {192, 192, 192};
    constexpr RGB GRAY = {128, 128, 128};
    constexpr RGB MAROON = {128, 0, 0};
    constexpr RGB OLIVE = {128, 128, 0};
    constexpr RGB GREEN = {0, 128, 0};
    constexpr RGB PURPLE = {128, 0, 128};
    constexpr RGB TEAL = {0, 128, 128};
    constexpr RGB NAVY = {0, 0, 128};
}
