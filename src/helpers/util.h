#ifndef BRICKSIM_UTIL_H
#define BRICKSIM_UTIL_H

#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include "ray.h"
#include "../types.h"

namespace util {

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

    //string functions
    std::string trim(const std::string &input);
    void asLower(const char *input, char *output, size_t length);
    std::string asLower(const std::string &string);
    void asUpper(const char *input, char *output, size_t length);
    std::string asUpper(const std::string &string);
    void toLowerInPlace(char* string);
    void toUpperInPlace(char* string);
    bool endsWith(std::string const &fullString, std::string const &ending);
    bool startsWith(std::string const &fullString, std::string const &start);
    bool endsWith(const char* fullString, const char* ending);
    bool startsWith(const char* fullString, const char* start);
    void replaceAll(std::string &str, const std::string &from, const std::string &to);
    std::string replaceChar(const std::string &str, char from, char to);
    std::string formatBytesValue(size_t bytes);
    bool containsIgnoreCase(const std::string &full, const std::string &sub);
    bool equalsAlphanum(const std::string& a, const std::string& b);

    //os functions
    void openDefaultBrowser(const std::string &link);

    //vector/glm functions
    void coutMat4(glm::mat4 mat);
    void coutVec(glm::vec4 vec);
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
    float calculateDistanceOfPointToLine(const glm::vec2& line_start, const glm::vec2& line_end, const glm::vec2& point);
    float calculateDistanceOfPointToLine(const glm::usvec2& line_start, const glm::usvec2& line_end, const glm::usvec2& point);
    struct NormalProjectionResult {
        glm::vec2 nearestPointOnLine;
        glm::vec2 projection;
        float projectionLength;
        float distancePointToLine;
        float lineLength;
    };
    NormalProjectionResult normalProjectionOnLineClamped(const glm::vec2 &lineStart, const glm::vec2 &lineEnd, const glm::vec2& point);
    struct ClosestLineBetweenTwoRaysResult {
        glm::vec3 pointOnA;
        glm::vec3 pointOnB;
        float distanceToPointA;
        float distanceToPointB;
        float distanceBetweenPoints;
    };
    ClosestLineBetweenTwoRaysResult closestLineBetweenTwoRays(const Ray3& a, const Ray3& b);

    // texture/image functions
    std::string translateBrickLinkColorNameToLDraw(std::string colorName);
    std::string translateLDrawColorNameToBricklink(std::string colorName);
    bool writeImage(const char *path, unsigned char *pixels, unsigned int width, unsigned int height, int channels = 3);

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
