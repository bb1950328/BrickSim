#pragma once

#include "../types.h"
#include <glm/glm.hpp>
#include <string>
#include <random>
//RGB is a macro in some windows header
#undef RGB

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

        color_component_t red, green, blue;
        [[nodiscard]] glm::vec3 asGlmVector() const;
        [[nodiscard]] std::string asHtmlCode() const;

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
    };

    class HSV {
    public:
        HSV() = default;
        explicit HSV(glm::vec3 vector);
        explicit HSV(RGB rgb);
        HSV(color_component_t hue, color_component_t saturation, color_component_t value);

        color_component_t hue, saturation, value;
        [[nodiscard]] glm::vec3 asGlmVector() const;
    };

    /**
     * the RNG is initialized with a static seed to get reproducible results
     * @return a random color. the saturation and the value of the HSV representation are always 255.
     */
    RGB getRandom();
}
