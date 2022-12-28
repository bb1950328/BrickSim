#pragma once

#include "../types.h"
#include <glm/glm.hpp>
#include <string>
#include <random>

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
        RGB() = default;

        explicit RGB(const std::string& htmlCode);
        explicit RGB(glm::vec3 vector);
        explicit RGB(const HSV& hsv);

        RGB(color_component_t red, color_component_t green, color_component_t blue);

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

        const static RGB BLACK;
        const static RGB WHITE;
        const static RGB RED;
        const static RGB LIME;
        const static RGB BLUE;
        const static RGB YELLOW;
        const static RGB CYAN;
        const static RGB MAGENTA;
        const static RGB SILVER;
        const static RGB GRAY;
        const static RGB MAROON;
        const static RGB OLIVE;
        const static RGB GREEN;
        const static RGB PURPLE;
        const static RGB TEAL;
        const static RGB NAVY;

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
    };

    /**
     * the RNG is initialized with a static seed to get reproducible results
     * @return a random color. the saturation and the value of the HSV representation are always 255.
     */
    RGB getRandom();
}
