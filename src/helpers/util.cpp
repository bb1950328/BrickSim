// util.cpp
// Created by bb1950328 on 20.09.20.
//

#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>
#include "util.h"
#include "../git_stats.h"

#ifdef _WIN32

#include <windows.h>

#endif

#include <stdlib.h>
#include <GL/gl.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

namespace util {
    std::string extend_home_dir(const std::string &input) {
        return extend_home_dir_path(input).string();
    }

    std::filesystem::path extend_home_dir_path(const std::string &input) {
        if (input[0] == '~' && (input[1] == '/' || input[1] == '\\')) {
            return std::filesystem::path(getenv(USER_ENV_VAR)) / std::filesystem::path(input.substr(2));
        } else {
            return std::filesystem::path(input);
        }
    }

    std::string trim(const std::string &input) {
        auto wsbefore = std::find_if_not(input.begin(), input.end(), [](int c) { return std::isspace(c); });
        auto wsafter = std::find_if_not(input.rbegin(), input.rend(), [](int c) { return std::isspace(c); }).base();
        return (wsafter <= wsbefore ? std::string() : std::string(wsbefore, wsafter));
    }

    std::string pathjoin(const std::list<std::string> &parts) {
        auto result = std::string();
        for (const auto &part : parts) {
            result.append(part);
            if (result.back() != PATH_SEPARATOR) {
                result.push_back(PATH_SEPARATOR);
            }
        }
        result.pop_back();//remove last separator
        std::replace(result.begin(), result.end(), PATH_SEPARATOR_FOREIGN, PATH_SEPARATOR);
        return result;
    }

    std::string as_lower(const std::string &string) {
        auto result = std::string();
        for (const auto &ch: string) {
            result.push_back(std::tolower(ch));
        }
        return result;
    }

    bool ends_with(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    bool starts_with(std::string const &fullString, std::string const &start) {
        return fullString.rfind(start, 0) == 0;
    }

    void cout_mat4(glm::mat4 mat) {/*
        std::cout << "⌈" << glm::to_string(mat[0]) << "⌉\n";
        std::cout << "|" << glm::to_string(mat[1]) << "|\n";
        std::cout << "|" << glm::to_string(mat[2]) << "|\n";
        std::cout << "⌊" << glm::to_string(mat[3]) << "⌋\n";*/
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    }

    void replaceAll(std::string &str, const std::string &from, const std::string &to) {
        //https://stackoverflow.com/a/3418285/8733066
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    std::string replaceChar(const std::string &str, char from, char to) {
        std::string result(str);
        if (from != to) {
            for (char &i : result) {
                if (i == from) {
                    i = to;
                }
            }
        }
        return result;
    }

    float biggest_value(glm::vec2 vector) {
        return std::max(vector.x, vector.y);
    }

    float biggest_value(glm::vec3 vector) {
        return std::max(std::max(vector.x, vector.y), vector.z);
    }

    float biggest_value(glm::vec4 vector) {
        return std::max(std::max(vector.x, vector.y), std::max(vector.z, vector.w));
    }

    unsigned long gcd(unsigned long a, unsigned long b) {
        //from https://www.geeksforgeeks.org/steins-algorithm-for-finding-gcd/
        if (a == 0)
            return b;
        if (b == 0)
            return a;

        unsigned long k;
        for (k = 0; (a | b) != 0 == 0; ++k) {
            a >>= 1u;
            b >>= 1u;
        }

        while ((a > 1) == 0) {
            a >>= 1u;
        }

        do {
            while ((b > 1) == 0) {
                b >>= 1u;
            }

            if (a > b) {
                std::swap(a, b);
            }

            b = (b - a);
        } while (b != 0);

        return a << k;
    }

    unsigned long lcm(unsigned long a, unsigned long b) {
        return a / gcd(a, b) * b;//https://stackoverflow.com/a/3154503/8733066
    }

    void open_default_browser(const std::string &link) {
#ifdef _WIN32
        ShellExecute(nullptr, "open", link.c_str(), nullptr, nullptr, SW_SHOWNORMAL);//todo testing
#endif
#ifdef __APPLE__
        std::string command = std::string("open ") + link;//todo testing
        std::cout << command << std::endl;
        system(command.c_str());
#elif __linux
        std::string command = std::string("xdg-open ") + link;
        std::cout << command << std::endl;
        system(command.c_str());
#endif
    }

    RGBcolor::RGBcolor(const std::string &htmlCode) {
        std::sscanf(htmlCode.c_str(), "#%2hx%2hx%2hx", &red, &green, &blue);
    }

    glm::vec3 triangleCentroid(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
        return (p1 + p2 + p3) / 3.0f;//todo check if this is mathematically correct
    }

    glm::vec3
    quadrilateralCentroid(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4) {
        return (p1 + p2 + p3 + p4) / 4.0f;//todo check if this is mathematically correct
    }

    bool doesTransformationInverseWindingOrder(const glm::mat4 &transformation) {
        glm::vec3 vec1 = transformation[0];
        glm::vec3 vec2 = transformation[1];
        glm::vec3 vec3 = transformation[2];
        glm::vec3 cross = glm::cross(vec1, vec2);
        return glm::dot(cross, vec3) < 0.0f;
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

    std::vector<std::string> getSystemInfo() {
        std::vector<std::string> result;
        const GLubyte *vendor = glGetString(GL_VENDOR);
        const GLubyte *renderer = glGetString(GL_RENDERER);
        result.push_back(std::string("sizeof(void*):\t") + std::to_string(sizeof(void *)) + " Bytes or " + std::to_string(sizeof(void *) * 8) + " Bits");
        result.push_back(std::string("sizeof(char):\t") + std::to_string(sizeof(char)) + " Bytes or " + std::to_string(sizeof(char) * 8) + " Bits");
        result.push_back(std::string("sizeof(int):\t") + std::to_string(sizeof(int)) + " Bytes or " + std::to_string(sizeof(int) * 8) + " Bits");
        result.push_back(std::string("sizeof(long):\t") + std::to_string(sizeof(long)) + " Bytes or " + std::to_string(sizeof(long) * 8) + " Bits");
        result.push_back(std::string("sizeof(float):\t") + std::to_string(sizeof(float)) + " Bytes or " + std::to_string(sizeof(float) * 8) + " Bits");
        result.push_back(std::string("sizeof(double):\t") + std::to_string(sizeof(double)) + " Bytes or " + std::to_string(sizeof(double) * 8) + " Bits");
        result.push_back(std::string("GPU Vendor:\t") + std::string(reinterpret_cast<const char *>(vendor)));
        result.push_back(std::string("GPU Renderer:\t") + std::string(reinterpret_cast<const char *>(renderer)));
        result.push_back(std::string("Git Commit Hash:\t")+git_stats::lastCommitHash);
        result.push_back(std::string("Dear ImGUI Version:\t")+IMGUI_VERSION);
        result.push_back(std::string("GLM Version:\t")+GLM_VERSION_MESSAGE);
        result.push_back(std::string("GLFW Version:\t")+std::to_string(GLFW_VERSION_MAJOR)+"."+std::to_string(GLFW_VERSION_MINOR)+"."+std::to_string(GLFW_VERSION_REVISION));
        return result;
    }

    float vector_sum(glm::vec2 vector) {
        return vector.x + vector.y;
    }

    float vector_sum(glm::vec3 vector) {
        return vector.x + vector.y + vector.z;
    }

    float vector_sum(glm::vec4 vector) {
        return vector.x + vector.y + vector.z + vector.w;
    }

    std::string formatBytesValue(size_t bytes) {
        double doubleBytes = bytes;
        static std::string bytePrefixes[] = {"B", "KB", "MB", "GB", "TB"};
        size_t prefixIndex = 0;
        while (doubleBytes > 1024) {
            prefixIndex++;
            doubleBytes /= 1024;
        }
        std::stringstream resultStream;
        resultStream << /*resultStream.precision(3) <<*/ doubleBytes << bytePrefixes[prefixIndex];
        return resultStream.str();
    }

    std::string RGBcolor::asHtmlCode() const {
        char buffer[8];
        snprintf(buffer, 8, "#%02x%02x%02x", red, green, blue);
        auto result = std::string(buffer);
        return result;
    }

    RGBcolor::RGBcolor(glm::vec3 vector) {
        red = vector.x * 255;
        green = vector.y * 255;
        blue = vector.z * 255;
    }

    glm::vec3 RGBcolor::asGlmVector() const {
        return glm::vec3(red / 255.0f, green / 255.0f, blue / 255.0f);
    }

    RGBcolor::RGBcolor(HSVcolor hsv) {
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

    RGBcolor::RGBcolor(unsigned short red, unsigned short green, unsigned short blue) : red(red), green(green), blue(blue) {

    }

    HSVcolor::HSVcolor(glm::vec3 vector) {
        hue = vector.x * 255;
        saturation = vector.y * 255;
        value = vector.z * 255;
    }

    glm::vec3 HSVcolor::asGlmVector() const {
        return glm::vec3(hue / 255.0f, saturation / 255.0f, value / 255.0f);
    }

    HSVcolor::HSVcolor(RGBcolor rgb) {
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
}