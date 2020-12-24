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
#include "../config.h"
#include "../lib/stb_image_write.h"
#include "../lib/stb_image.h"
#include "../controller.h"

#ifdef _WIN32

#include <windows.h>

#endif

#include <stdlib.h>
#include <GL/gl.h>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <mutex>

namespace util {
    namespace {
        unsigned int copyTextureToVram(int imgWidth, int imgHeight, int nrChannels, const unsigned char *data) {
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            unsigned int textureId;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);

            GLenum format;
            if (nrChannels == 1) {
                format = GL_RED;
            } else if (nrChannels == 3) {
                format = GL_RGB;
            } else if (nrChannels == 4) {
                format = GL_RGBA;
            } else {
                std::cout << "WARNING: image has a weird number of channels: " << nrChannels << std::endl;
                format = GL_RGB;
            }
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, format, imgWidth, imgHeight, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            return textureId;
        }
    }

    std::string extendHomeDir(const std::string &input) {
        return extendHomeDirPath(input).string();
    }

    std::filesystem::path extendHomeDirPath(const std::string &input) {
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

    std::string asLower(const std::string &string) {
        auto result = std::string();
        for (const auto &ch: string) {
            result.push_back(std::tolower(ch));
        }
        return result;
    }

    bool endsWith(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    bool startsWith(std::string const &fullString, std::string const &start) {
        return fullString.rfind(start, 0) == 0;
    }

    void coutMat4(glm::mat4 mat) {/*
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

    float biggestValue(glm::vec2 vector) {
        return std::max(vector.x, vector.y);
    }

    float biggestValue(glm::vec3 vector) {
        return std::max(std::max(vector.x, vector.y), vector.z);
    }

    float biggestValue(glm::vec4 vector) {
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

    void openDefaultBrowser(const std::string &link) {
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
        std::sscanf(htmlCode.c_str(), "#%2hhx%2hhx%2hhx", &red, &green, &blue);
    }

    glm::vec3 triangleCentroid(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
        return (p1 + p2 + p3) / 3.0f;
    }

    glm::vec3 quadrilateralCentroid(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4) {
        return (p1 + p2 + p3 + p4) / 4.0f;
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
        //result.push_back(std::string("GLM Version:\t")+GLM_VERSION_MESSAGE);//todo
        result.push_back(std::string("GLFW Version:\t")+std::to_string(GLFW_VERSION_MAJOR)+"."+std::to_string(GLFW_VERSION_MINOR)+"."+std::to_string(GLFW_VERSION_REVISION));
        return result;
    }

    float vectorSum(glm::vec2 vector) {
        return vector.x + vector.y;
    }

    float vectorSum(glm::vec3 vector) {
        return vector.x + vector.y + vector.z;
    }

    float vectorSum(glm::vec4 vector) {
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

    RGBcolor::RGBcolor(color_component_t red, color_component_t green, color_component_t blue) : red(red), green(green), blue(blue) {

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


    bool memeqzero(const void *data, size_t length) {
        //from https://github.com/rustyrussell/ccan/blob/master/ccan/mem/mem.c#L92
        const auto *p = static_cast<const unsigned char *>(data);
        size_t len;

        /* Check first 16 bytes manually */
        for (len = 0; len < 16; len++) {
            if (!length)
                return true;
            if (*p)
                return false;
            p++;
            length--;
        }

        /* Now we know that's zero, memcmp with self. */
        return memcmp(data, p, length) == 0;
    }

    std::string fileToString(const std::filesystem::path& path) {
        https://stackoverflow.com/a/2602258/8733066
        std::ifstream t(path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }

    glm::vec2 minForEachComponent(const glm::vec2 &a, const glm::vec2 &b) {
        return {std::min(a.x, b.x), std::min(a.y, b.y)};
    }

    glm::vec3 minForEachComponent(const glm::vec3 &a, const glm::vec3 &b) {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
    }

    glm::vec4 minForEachComponent(const glm::vec4 &a, const glm::vec4 &b) {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)};
    }

    std::string translateBrickLinkColorNameToLDraw(std::string colorName) {
        colorName = util::replaceChar(colorName, ' ', '_');
        colorName = util::replaceChar(colorName, '-', '_');
        util::replaceAll(colorName, "Gray", "Grey");
        return colorName;
    }

    std::string translateLDrawColorNameToBricklink(std::string colorName) {
        colorName = util::replaceChar(colorName, '_', ' ');
        util::replaceAll(colorName, "Grey", "Gray");
        return colorName;
    }

    bool equalsAlphanum(std::string a, std::string b) {
        auto itA = a.cbegin();
        auto itB = b.cbegin();
        while (itA != a.cend() && itB != b.cend()) {
            while (itA != a.cend() && !std::isalnum(*itA)) {
                ++itA;
            }
            while (itB != b.cend() && !std::isalnum(*itB)) {
                ++itB;
            }
            if ((itA==a.cend())!=(itB==b.cend())) {//
                return false;
            }
            if (itA!=a.cend() && *itA != *itB) {
                return false;
            }
            ++itA;
            ++itB;
        }
        return true;
    }

    std::filesystem::path withoutBasePath(const std::filesystem::path &path, const std::filesystem::path &basePath) {
        //todo this can be more efficient
        std::string result = path.string();
        replaceAll(result, basePath.string(), "");
        if (result[0]==PATH_SEPARATOR) {
            result.erase(0, 1);
        }
        return result;
    }

    bool writeImage(const char *path, unsigned char* pixels, unsigned int width, unsigned int height, int channels) {
        auto path_lower = util::asLower(path);
        stbi_flip_vertically_on_write(true);
        if (util::endsWith(path_lower, ".png")) {
            return stbi_write_png(path, width, height, channels, pixels, width * channels)!=0;
        } else if (util::endsWith(path_lower, ".jpg") || util::endsWith(path, ".jpeg")) {
            const int quality = std::min(100, std::max(5, (int) config::getInt(config::JPG_SCREENSHOT_QUALITY)));
            return stbi_write_jpg(path, width, height, channels, pixels, quality)!=0;
        } else if (util::endsWith(path_lower, ".bmp")) {
            return stbi_write_bmp(path, width, height, channels, pixels)!=0;
        } else if (util::endsWith(path_lower, ".tga")) {
            return stbi_write_tga(path, width, height, channels, pixels)!=0;
        } else {
            return false;
        }
    }

    bool containsIgnoreCase(const std::string &full, const std::string &sub) {
        return std::search(
                full.begin(), full.end(),
                sub.begin(), sub.end(),
                [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
        ) != full.end();
    }

    unsigned int loadTextureFromFile(const std::filesystem::path &image) {
        int imgWidth, imgHeight, nrChannels;
        unsigned int textureId;
        unsigned char *data = stbi_load(image.string().c_str(), &imgWidth, &imgHeight, &nrChannels, 3);
        if (!data) {
            throw std::invalid_argument("texture not read successfully: " + image.string());
        }
        textureId = copyTextureToVram(imgWidth, imgHeight, nrChannels, data);
        stbi_image_free(data);
        return textureId;
    }

    unsigned int loadTextureFromMemory(const unsigned char* fileData, unsigned int dataSize) {
        int imgWidth, imgHeight, nrChannels;
        unsigned int textureId;
        unsigned char *data = stbi_load_from_memory(fileData, dataSize, &imgWidth, &imgHeight, &nrChannels, 3);
        if (!data) {
            throw std::invalid_argument("texture not read successfully from memory: " + std::to_string((unsigned long) fileData));
        }
        textureId = copyTextureToVram(imgWidth, imgHeight, nrChannels, data);
        stbi_image_free(data);
        return textureId;
    }
}