

#include "color.h"
#include "glm/glm.hpp"

namespace color {
    RGB::RGB(const std::string &htmlCode) {
        std::sscanf(htmlCode.c_str(), "#%2hhx%2hhx%2hhx", &red, &green, &blue);
    }
    RGB::RGB(glm::vec3 vector) {
        red = vector.x * 255;
        green = vector.y * 255;
        blue = vector.z * 255;
    }

    RGB::RGB(color_component_t red, color_component_t green, color_component_t blue) : red(red), green(green), blue(blue) {}

    RGB::RGB(const HSV& hsv) {
        if (hsv.saturation == 0) {
            red = hsv.value;
            green = hsv.value;
            blue = hsv.value;
        } else {
            float h = hsv.hue / 255.0f;
            float s = hsv.saturation / 255.0f;
            float v = hsv.value / 255.0f;
            auto i = (int) std::floor(h * 6);
            auto f = h * 6 - i;
            auto p = v * (1.0f - s);
            auto q = v * (1.0f - s * f);
            auto t = v * (1.0f - s * (1.0f - f));
            switch (i % 6) {
                case 0:
                    red = v * 255;
                    green = t * 255;
                    blue = p * 255;
                    break;
                case 1:
                    red = q * 255;
                    green = v * 255;
                    blue = p * 255;
                    break;
                case 2:
                    red = p * 255;
                    green = v * 255;
                    blue = t * 255;
                    break;
                case 3:
                    red = p * 255;
                    green = q * 255;
                    blue = v * 255;
                    break;
                case 4:
                    red = t * 255;
                    green = p * 255;
                    blue = v * 255;
                    break;
                case 5:
                    red = v * 255;
                    green = p * 255;
                    blue = q * 255;
                    break;
                default:
                    break;//shouldn't get here
            }
        }
    }

    std::string RGB::asHtmlCode() const {
        char buffer[8];
        snprintf(buffer, 8, "#%02x%02x%02x", red, green, blue);
        auto result = std::string(buffer);
        return result;
    }

    glm::vec3 RGB::asGlmVector() const {
        return glm::vec3(red / 255.0f, green / 255.0f, blue / 255.0f);
    }

    const RGB RGB::BLACK{0, 0, 0};
    const RGB RGB::WHITE{255, 255, 255};
    const RGB RGB::RED{255, 0, 0};
    const RGB RGB::LIME{0, 255, 0};
    const RGB RGB::BLUE{0, 0, 255};
    const RGB RGB::YELLOW{255, 255, 0};
    const RGB RGB::CYAN{0, 255, 255};
    const RGB RGB::MAGENTA{255, 0, 255};
    const RGB RGB::SILVER{192, 192, 192};
    const RGB RGB::GRAY{128, 128, 128};
    const RGB RGB::MAROON{128, 0, 0};
    const RGB RGB::OLIVE{128, 128, 0};
    const RGB RGB::GREEN{0, 128, 0};
    const RGB RGB::PURPLE{128, 0, 128};
    const RGB RGB::TEAL{0, 128, 128};
    const RGB RGB::NAVY{0, 0, 128};

    HSV::HSV(glm::vec3 vector) {
        hue = vector.x * 255;
        saturation = vector.y * 255;
        value = vector.z * 255;
    }

    HSV::HSV(RGB rgb) {
        auto maxc = std::max(std::max(rgb.red, rgb.green), rgb.blue);
        auto minc = std::min(std::min(rgb.red, rgb.green), rgb.blue);
        value = maxc;
        if (maxc != minc) {
            const auto maxmindiff = maxc - minc;
            saturation = maxmindiff * 1.0f / maxc;
            auto rc = (maxc - rgb.red) * 1.0f / maxmindiff;
            auto gc = (maxc - rgb.green) * 1.0f / maxmindiff;
            auto bc = (maxc - rgb.blue) * 1.0f / maxmindiff;
            float h;
            if (rgb.red == maxc) {
                h = bc - gc;
            } else if (rgb.green == maxc) {
                h = 2.0f + rc - bc;
            } else {
                h = 4.0f + gc - rc;
            }
            hue = (((h / 255 / 6.0f) - (int) (h / 255 / 6.0f)) * 255.0f);
        }
    }

    glm::vec3 HSV::asGlmVector() const {
        return glm::vec3(hue / 255.0f, saturation / 255.0f, value / 255.0f);
    }

    glm::vec3 convertIntToColorVec3(unsigned int value) {
        unsigned char bluePart = value & 0xffu;//blue first is intended
        value >>= 8u;
        unsigned char greenPart = value & 0xffu;
        value >>= 8u;
        unsigned char redPart = value & 0xffu;
        return glm::vec3(redPart / 255.0f, greenPart / 255.0f, bluePart / 255.0f);
    }

    unsigned int getIntFromColor(unsigned char red, unsigned char green, unsigned char blue) {
        unsigned int result = ((unsigned int) red) << 16u | ((unsigned int) green) << 8u | blue;
        return result;
    }
}