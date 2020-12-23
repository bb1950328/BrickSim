// util.h
// Created by bb1950328 on 20.09.20.
//

#ifndef BRICKSIM_UTIL_H
#define BRICKSIM_UTIL_H

#include <string>
#include <vector>
#include <list>
#include <filesystem>
#include <ostream>
#include <glm/glm.hpp>

namespace util {
    namespace {
        unsigned int copyTextureToVram(int imgWidth, int imgHeight, int nrChannels, const unsigned char *data);
    }

#if _WIN32
    const char *const USER_ENV_VAR = "USERPROFILE";
    const char PATH_SEPARATOR = '\\';
    const char PATH_SEPARATOR_FOREIGN = '/';
#else
    const char *const USER_ENV_VAR = "HOME";
    const char PATH_SEPARATOR = '/';
    const char PATH_SEPARATOR_FOREIGN = '\\';
#endif

    std::string extendHomeDir(const std::string &input);
    std::filesystem::path extendHomeDirPath(const std::string &input);
    std::string trim(const std::string& input);
    std::string pathjoin(const std::list<std::string>& parts);
    std::string asLower(const std::string& string);
    bool endsWith(std::string const &fullString, std::string const &ending);
    bool startsWith(std::string const &fullString, std::string const &start);
    void coutMat4(glm::mat4 mat);
    void replaceAll(std::string& str, const std::string& from, const std::string& to);
    std::string replaceChar(const std::string& str, char from, char to);
    unsigned long gcd(unsigned long a, unsigned long b);
    unsigned long lcm(unsigned long a, unsigned long b);
    float biggestValue(glm::vec2 vector);
    float biggestValue(glm::vec3 vector);
    float biggestValue(glm::vec4 vector);
    float vectorSum(glm::vec2 vector);
    float vectorSum(glm::vec3 vector);
    float vectorSum(glm::vec4 vector);
    void openDefaultBrowser(const std::string& link);
    glm::vec3 triangleCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
    glm::vec3 quadrilateralCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4);
    bool doesTransformationInverseWindingOrder(const glm::mat4& transformation);
    glm::vec3 convertIntToColorVec3(unsigned int value);
    unsigned int getIntFromColor(unsigned char red, unsigned char green, unsigned char blue);
    std::string formatBytesValue(size_t bytes);
    std::string fileToString(const std::filesystem::path& path);
    glm::vec2 minForEachComponent(const glm::vec2& a, const glm::vec2& b);
    glm::vec3 minForEachComponent(const glm::vec3& a, const glm::vec3& b);
    glm::vec4 minForEachComponent(const glm::vec4& a, const glm::vec4& b);

    class HSVcolor;
    class RGBcolor {
    public:
        RGBcolor() = default;

        explicit RGBcolor(const std::string& htmlCode);
        explicit RGBcolor(glm::vec3 vector);
        explicit RGBcolor(HSVcolor hsv);

        RGBcolor(unsigned short red, unsigned short green, unsigned short blue);

        unsigned short red, green, blue;
        [[nodiscard]] glm::vec3 asGlmVector() const;
        [[nodiscard]] std::string asHtmlCode() const;
    };

    class HSVcolor {
    public:
        HSVcolor() = default;
        explicit HSVcolor(glm::vec3 vector);
        explicit HSVcolor(RGBcolor rgb);

        unsigned short hue, saturation, value;
        [[nodiscard]] glm::vec3 asGlmVector() const;
    };

    std::vector<std::string> getSystemInfo();
    bool memeqzero(const void *data, size_t length);
    std::string translateBrickLinkColorNameToLDraw(std::string colorName);
    std::string translateLDrawColorNameToBricklink(std::string colorName);
    bool equalsAlphanum(std::string a, std::string b);
    std::filesystem::path withoutBasePath(const std::filesystem::path& path, const std::filesystem::path& basePath);
    bool writeImage(const char* path, unsigned char* pixels, unsigned int width, unsigned int height, int channels=3);
    bool containsIgnoreCase(const std::string& full, const std::string& sub);
    unsigned int loadTextureFromFile(const std::filesystem::path& image);
    unsigned int loadTextureFromMemory(const unsigned char* fileData, unsigned int dataSize);
}
#endif //BRICKSIM_UTIL_H
