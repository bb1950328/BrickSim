

#ifndef BRICKSIM_UTIL_H
#define BRICKSIM_UTIL_H

#include <string>
#include <vector>
#include <list>
#include <filesystem>
#include <ostream>
#include <glm/glm.hpp>
#include "../types.h"

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

    // path functions
    std::string extendHomeDir(const std::string &input);
    std::string replaceHomeDir(const std::string &input);
    std::filesystem::path extendHomeDirPath(const std::string &input);
    std::filesystem::path withoutBasePath(const std::filesystem::path &path, const std::filesystem::path &basePath);
    std::string pathjoin(const std::list<std::string> &parts);

    //string functions
    std::string trim(const std::string &input);
    std::string asLower(const std::string &string);
    void toUpperInPlace(char* string);
    bool endsWith(std::string const &fullString, std::string const &ending);
    bool startsWith(std::string const &fullString, std::string const &start);
    void replaceAll(std::string &str, const std::string &from, const std::string &to);
    std::string replaceChar(const std::string &str, char from, char to);
    std::string formatBytesValue(size_t bytes);
    std::string fileToString(const std::filesystem::path &path);
    bool containsIgnoreCase(const std::string &full, const std::string &sub);
    bool equalsAlphanum(std::string a, std::string b);

    //number functions
    unsigned long gcd(unsigned long a, unsigned long b);
    unsigned long lcm(unsigned long a, unsigned long b);

    //os functions
    void openDefaultBrowser(const std::string &link);
    std::vector<std::pair<const char *, std::string>> getSystemInfo();

    //vector/glm functions
    void coutMat4(glm::mat4 mat);
    float biggestValue(glm::vec2 vector);
    float biggestValue(glm::vec3 vector);
    float biggestValue(glm::vec4 vector);
    float vectorSum(glm::vec2 vector);
    float vectorSum(glm::vec3 vector);
    float vectorSum(glm::vec4 vector);
    glm::vec2 minForEachComponent(const glm::vec2 &a, const glm::vec2 &b);
    glm::vec3 minForEachComponent(const glm::vec3 &a, const glm::vec3 &b);
    glm::vec4 minForEachComponent(const glm::vec4 &a, const glm::vec4 &b);

    //geometry functions
    glm::vec3 triangleCentroid(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3);
    glm::vec3 quadrilateralCentroid(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4);
    bool doesTransformationInverseWindingOrder(const glm::mat4 &transformation);


    //color functions
    glm::vec3 convertIntToColorVec3(unsigned int value);
    unsigned int getIntFromColor(unsigned char red, unsigned char green, unsigned char blue);

    class HSVcolor;

    class RGBcolor {
    public:
        RGBcolor() = default;

        explicit RGBcolor(const std::string &htmlCode);
        explicit RGBcolor(glm::vec3 vector);
        explicit RGBcolor(const HSVcolor& hsv);

        RGBcolor(color_component_t red, color_component_t green, color_component_t blue);

        color_component_t red, green, blue;
        [[nodiscard]] glm::vec3 asGlmVector() const;
        [[nodiscard]] std::string asHtmlCode() const;
    };

    class HSVcolor {
    public:
        HSVcolor() = default;
        explicit HSVcolor(glm::vec3 vector);
        explicit HSVcolor(RGBcolor rgb);

        color_component_t hue, saturation, value;
        [[nodiscard]] glm::vec3 asGlmVector() const;
    };


    // texture/image functions
    struct TextureLoadResult {
        unsigned int textureId;
        int width;
        int height;
        int nrChannels;
    };

    std::string translateBrickLinkColorNameToLDraw(std::string colorName);
    std::string translateLDrawColorNameToBricklink(std::string colorName);
    bool writeImage(const char *path, unsigned char *pixels, unsigned int width, unsigned int height, int channels = 3);

    TextureLoadResult loadTextureFromFile(const std::filesystem::path &image);
    TextureLoadResult loadTextureFromMemory(const unsigned char *fileData, unsigned int dataSize);

    bool isStbiFlipVertically();
    void setStbiFlipVertically(bool value);

    bool memeqzero(const void *data, size_t length);

    constexpr int RESPONSE_CODE_FROM_CACHE = 1001;

    /**
     * @param url
     * @param useCache if db::requestCache should be used
     * @param sizeLimit stop download after sizeLimit bytes, default never stop
     * @param progressFunc int(void* clientp, long downloadTotal, long downloadNow, long uploadTotal, long uploadNow) // if return value is != 0, transfer stops
     * @return (responseCode, responseString)
     */
    std::pair<int, std::string> requestGET(const std::string &url, bool useCache = true, size_t sizeLimit = 0, int (*progressFunc)(void *, long, long, long, long) = nullptr);

    std::string readFileToString(const std::filesystem::path& path);
}



#endif //BRICKSIM_UTIL_H
