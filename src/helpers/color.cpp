#include "color.h"
#include <spdlog/fmt/fmt.h>

namespace bricksim::color {
    RGB::RGB(const std::string& htmlCode) {
        std::sscanf(htmlCode.c_str(), "#%2hhx%2hhx%2hhx", &red, &green, &blue);
    }
    RGB::RGB(glm::vec3 vector) {
        red = static_cast<color_component_t>(std::clamp(vector.x, 0.f, 1.f) * 255 + .5f);
        green = static_cast<color_component_t>(std::clamp(vector.y, 0.f, 1.f) * 255 + .5f);
        blue = static_cast<color_component_t>(std::clamp(vector.z, 0.f, 1.f) * 255 + .5f);
    }

    RGB::RGB(const HSV& hsv) {
        if (hsv.saturation == 0) {
            red = hsv.value;
            green = hsv.value;
            blue = hsv.value;
        } else {
            float h = static_cast<float>(hsv.hue) / 255.f;
            float s = static_cast<float>(hsv.saturation) / 255.f;
            float v = static_cast<float>(hsv.value) / 255.f;
            auto i = static_cast<int8_t>(std::floor(h * 6));
            auto f = h * 6 - static_cast<float>(i);
            auto p = v * (1.0f - s);
            auto q = v * (1.0f - s * f);
            auto t = v * (1.0f - s * (1.0f - f));
            switch (i % 6) {
                case 0:
                    assignFloat(v, t, p);
                    break;
                case 1:
                    assignFloat(q, v, p);
                    break;
                case 2:
                    assignFloat(p, v, t);
                    break;
                case 3:
                    assignFloat(p, q, v);
                    break;
                case 4:
                    assignFloat(t, p, v);
                    break;
                case 5:
                    assignFloat(v, p, q);
                    break;
                default:
                    break;//shouldn't get here
            }
        }
    }

    std::string RGB::asHtmlCode() const {
        return fmt::format("#{:02x}{:02x}{:02x}", red, green, blue);
    }

    glm::vec3 RGB::asGlmVector() const {
        return {static_cast<float>(red) / 255.0f,
                static_cast<float>(green) / 255.0f,
                static_cast<float>(blue) / 255.0f};
    }

    void RGB::assignFloat(float r, float g, float b) {
        red = static_cast<color_component_t>(r * 255);
        green = static_cast<color_component_t>(g * 255);
        blue = static_cast<color_component_t>(b * 255);
    }
    bool RGB::operator==(const RGB& rhs) const {
        return red == rhs.red && green == rhs.green && blue == rhs.blue;
    }
    bool RGB::operator!=(const RGB& rhs) const {
        return !(rhs == *this);
    }
    bool RGB::operator<(const RGB& rhs) const {
        return HSV(*this) < HSV(rhs);
    }
    bool RGB::operator>(const RGB& rhs) const {
        return rhs < *this;
    }
    bool RGB::operator<=(const RGB& rhs) const {
        return !(rhs < *this);
    }
    bool RGB::operator>=(const RGB& rhs) const {
        return !(*this < rhs);
    }

    HSV::HSV(glm::vec3 vector) {
        hue = static_cast<color_component_t>(std::clamp(vector.x, 0.f, 1.f) * 255 + .5f);
        saturation = static_cast<color_component_t>(std::clamp(vector.y, 0.f, 1.f) * 255 + .5f);
        value = static_cast<color_component_t>(std::clamp(vector.z, 0.f, 1.f) * 255 + .5f);
    }

    HSV::HSV(RGB rgb) {
        auto maxc = std::max(std::max(rgb.red, rgb.green), rgb.blue);
        auto minc = std::min(std::min(rgb.red, rgb.green), rgb.blue);
        value = maxc;
        if (maxc != minc) {
            const auto maxmindiff = static_cast<float>(maxc - minc);
            saturation = static_cast<color_component_t>(maxmindiff / static_cast<float>(maxc));
            auto rc = static_cast<float>(maxc - rgb.red) / maxmindiff;
            auto gc = static_cast<float>(maxc - rgb.green) / maxmindiff;
            auto bc = static_cast<float>(maxc - rgb.blue) / maxmindiff;
            float h;
            if (rgb.red == maxc) {
                h = bc - gc;
            } else if (rgb.green == maxc) {
                h = 2.f + rc - bc;
            } else {
                h = 4.f + gc - rc;
            }
            hue = static_cast<color_component_t>(((h / 255.f / 6.f) - std::floor(h / 255.f / 6.f)) * 255.f);
        }
    }

    glm::vec3 HSV::asGlmVector() const {
        return {static_cast<float>(hue) / 255.f,
                static_cast<float>(saturation) / 255.f,
                static_cast<float>(value) / 255.f};
    }

    HSV::HSV(color_component_t hue, color_component_t saturation, color_component_t value) :
        hue(hue), saturation(saturation), value(value) {}
    bool HSV::operator==(const HSV& rhs) const {
        return hue == rhs.hue && saturation == rhs.saturation && value == rhs.value;
    }
    bool HSV::operator!=(const HSV& rhs) const {
        return !(rhs == *this);
    }
    bool HSV::operator<(const HSV& rhs) const {
        if (hue < rhs.hue)
            return true;
        if (rhs.hue < hue)
            return false;
        if (saturation < rhs.saturation)
            return true;
        if (rhs.saturation < saturation)
            return false;
        return value < rhs.value;
    }
    bool HSV::operator>(const HSV& rhs) const {
        return rhs < *this;
    }
    bool HSV::operator<=(const HSV& rhs) const {
        return !(rhs < *this);
    }
    bool HSV::operator>=(const HSV& rhs) const {
        return !(*this < rhs);
    }

    glm::vec3 convertIntToColorVec3(unsigned int value) {
        unsigned char bluePart = value & 0xffu;//blue first is intended
        value >>= 8u;
        unsigned char greenPart = value & 0xffu;
        value >>= 8u;
        unsigned char redPart = value & 0xffu;
        return {static_cast<float>(redPart) / 255.f,
                static_cast<float>(greenPart) / 255.f,
                static_cast<float>(bluePart) / 255.f};
    }

    unsigned int getIntFromColor(unsigned char red, unsigned char green, unsigned char blue) {
        unsigned int result = ((unsigned int)red) << 16u | ((unsigned int)green) << 8u | blue;
        return result;
    }

    RGB getRandom() {
        static std::mt19937 rng(123456789uL);// NOLINT(cert-msc51-cpp)
        return RGB(HSV(rng() & 0xff, 0xff, 0xff));
    }
}
