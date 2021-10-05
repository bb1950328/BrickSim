#pragma once

#include "../types.h"
#include "ray.h"
#include <filesystem>
#include <glm/glm.hpp>
#include <optional>
#include <string>

namespace bricksim::util {

#if _WIN32
    const char* const USER_ENV_VAR = "USERPROFILE";
    const char PATH_SEPARATOR = '\\';
    const char PATH_SEPARATOR_FOREIGN = '/';
#else
    const char* const USER_ENV_VAR = "HOME";
    const char PATH_SEPARATOR = '/';
    const char PATH_SEPARATOR_FOREIGN = '\\';
#endif

    // path functions
    std::string extendHomeDir(const std::string& input);
    std::string replaceHomeDir(const std::string& input);
    std::filesystem::path extendHomeDirPath(const std::string& input);
    std::filesystem::path withoutBasePath(const std::filesystem::path& path, const std::filesystem::path& basePath);

    //string functions
    std::string trim(const std::string& input);
    void asLower(const char* input, char* output, size_t length);
    std::string asLower(const std::string& string);
    void asUpper(const char* input, char* output, size_t length);
    std::string asUpper(const std::string& string);
    void toLowerInPlace(char* string);
    void toUpperInPlace(char* string);
    bool endsWith(std::string const& fullString, std::string const& ending);
    bool startsWith(std::string const& fullString, std::string const& start);
    bool endsWith(const char* fullString, const char* ending);
    bool startsWith(const char* fullString, const char* start);
    void replaceAll(std::string& str, const std::string& from, const std::string& to);
    std::string replaceChar(const std::string& str, char from, char to);
    std::string formatBytesValue(uint64_t bytes);
    bool containsIgnoreCase(const std::string& full, const std::string& sub);
    bool equalsAlphanum(const std::string& a, const std::string& b);

    //os functions
    void openDefaultBrowser(const std::string& link);

    template<glm::length_t L, typename T, glm::qualifier Q>
    T biggestValue(glm::vec<L, T, Q> vector) {
        T max = vector[0];
        for (int i = 1; i < L; ++i) {
            max = std::max(max, vector[i]);
        }
        return max;
    }

    template<glm::length_t L, typename T, glm::qualifier Q>
    T vectorSum(glm::vec<L, T, Q> vector) {
        T sum = 0;
        for (int i = 0; i < L; ++i) {
            sum += vector[i];
        }
        return sum;
    }

    template<glm::length_t L, typename T, glm::qualifier Q>
    glm::vec<L, T, Q> minForEachComponent(const glm::vec<L, T, Q>& a, const glm::vec<L, T, Q>& b) {
        glm::vec<L, T, Q> res;
        for (int i = 0; i < L; ++i) {
            res[i] = std::min(a[i], b[i]);
        }
        return res;
    }

    template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q>
    bool matEpsilonEqual(const glm::mat<L1, L2, T, Q>& mat1, const glm::mat<L1, L2, T, Q>& mat2, T epsilon) {
        bool result = true;
        for (glm::length_t i = 0; i < L1; ++i) {
            result &= glm::all(glm::epsilonEqual(mat1[0], mat2[0], epsilon));
        }
        return result;
    }

    struct DecomposedTransformation {
        glm::quat orientation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::vec3 translation;
        glm::vec3 scale;
        [[nodiscard]] glm::mat4 orientationAsMat4() const;
        [[nodiscard]] glm::mat4 translationAsMat4() const;
        [[nodiscard]] glm::mat4 scaleAsMat4() const;
    };
    DecomposedTransformation decomposeTransformationToStruct(const glm::mat4& transformation);

    //geometry functions
    glm::vec3 triangleCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
    glm::vec3 quadrilateralCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4);
    bool doesTransformationInverseWindingOrder(const glm::mat4& transformation);
    float calculateDistanceOfPointToLine(const glm::vec2& line_start, const glm::vec2& line_end, const glm::vec2& point);
    float calculateDistanceOfPointToLine(const glm::usvec2& line_start, const glm::usvec2& line_end, const glm::usvec2& point);
    struct NormalProjectionResult {
        glm::vec2 nearestPointOnLine;
        glm::vec2 projection;
        float projectionLength;
        float distancePointToLine;
        float lineLength;
    };
    NormalProjectionResult normalProjectionOnLineClamped(const glm::vec2& lineStart, const glm::vec2& lineEnd, const glm::vec2& point);
    struct ClosestLineBetweenTwoRaysResult {
        glm::vec3 pointOnA;
        glm::vec3 pointOnB;
        float distanceToPointA;
        float distanceToPointB;
        float distanceBetweenPoints;
    };
    ClosestLineBetweenTwoRaysResult closestLineBetweenTwoRays(const Ray3& a, const Ray3& b);

    std::optional<glm::vec3> rayPlaneIntersection(const Ray3& ray, const Ray3& planeNormal);
    std::optional<glm::vec3> linePlaneIntersection(const glm::vec3& lineP0, const glm::vec3& lineP1, const Ray3& planeNormal);

    glm::quat quaternionRotationFromOneVectorToAnother(const glm::vec3& v1, const glm::vec3& v2);
    glm::vec3 getAnyPerpendicularVector(const glm::vec3& v);

    float getAngleBetweenThreePointsUnsigned(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
    float getAngleBetweenThreePointsSigned(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& planeNormal);

    float getDistanceBetweenPointAndPlane(const Ray3& planeNormal, const glm::vec3& point);

    bool isPointInTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& point);

    class Plane3dTo2dConverter {
        glm::vec3 origin, xDir, yDir, planeNormal;

    public:
        Plane3dTo2dConverter(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
        glm::vec2 convert3dTo2d(const glm::vec3& pointOnPlane);
        glm::vec3 convert2dTo3d(const glm::vec2& coord);
    };

    /**
     * polygons are in CCW order
     */
    std::vector<glm::vec2> sutherlandHogmanPolygonClipping(const std::vector<glm::vec2>& subjectPolygon, const std::vector<glm::vec2>& clipPolygon);

    float getSignedPolygonArea(const std::vector<glm::vec2>& polygon);
    bool is2dPolygonClockwise(const std::vector<glm::vec2>& polygon);

    template<glm::length_t L>
    int findPointInPolygonEpsilon(std::vector<glm::vec<L, float, glm::defaultp>> poly, glm::vec<L, float, glm::defaultp> point) {
        for (int i = 0; i < poly.size(); ++i) {
            if (vectorSum(glm::abs(poly[i] - point)) < 0.0003f) {
                return i;
            }
        }
        return -1;
    }

    /**
     * WARNING: this function is horribly inefficient because it's brute force
     */
    template<glm::length_t L>
    std::vector<std::vector<glm::vec<L, float, glm::defaultp>>> joinTrianglesToPolygon(std::vector<std::array<glm::vec<L, float, glm::defaultp>, 3>> triangles) {
        static_assert(L == 2 || L == 3, "this only works for 2D and 3D coordinates");
        typedef glm::vec<L, float, glm::defaultp> point;
        std::vector<std::vector<point>> result;
        while (!triangles.empty()) {
            std::vector<point> currentPoly(triangles.back().begin(), triangles.back().end());
            triangles.pop_back();
            bool foundOne;
            do {
                foundOne = false;
                for (auto tri = triangles.begin(); tri < triangles.end();) {
                    const auto i0 = findPointInPolygonEpsilon(currentPoly, (*tri)[0]);
                    const auto i1 = findPointInPolygonEpsilon(currentPoly, (*tri)[1]);
                    const auto i2 = findPointInPolygonEpsilon(currentPoly, (*tri)[2]);
                    point additionalCoord;
                    int ia, ib;
                    foundOne = true;
                    if (i0 >= 0 && i1 >= 0) {
                        additionalCoord = (*tri)[2];
                        ia = i0;
                        ib = i1;
                    } else if (i0 >= 0 && i2 >= 0) {
                        additionalCoord = (*tri)[1];
                        ia = i0;
                        ib = i2;
                    } else if (i1 >= 0 && i2 >= 0) {
                        additionalCoord = (*tri)[0];
                        ia = i1;
                        ib = i2;
                    } else {
                        foundOne = false;
                    }
                    if (foundOne && abs(ia - ib) == 1) {
                        currentPoly.insert(currentPoly.begin() + std::max(ia, ib), additionalCoord);
                        tri = triangles.erase(tri);
                        break;
                    } else if (foundOne && std::min(ia, ib) == 0 && std::max(ia, ib) == currentPoly.size() - 1) {
                        currentPoly.push_back(additionalCoord);
                        tri = triangles.erase(tri);
                        break;
                    } else {
                        foundOne = false;
                        ++tri;
                    }
                }
            } while (foundOne);
            if constexpr (L == 2) {
                if (is2dPolygonClockwise(currentPoly)) {
                    std::reverse(currentPoly.begin(), currentPoly.end());
                }
            }
            result.push_back(currentPoly);
        }
        return result;
    }

    // texture/image functions
    std::string translateBrickLinkColorNameToLDraw(std::string colorName);
    std::string translateLDrawColorNameToBricklink(std::string colorName);
    bool writeImage(const char* path, unsigned char* pixels, unsigned int width, unsigned int height, int channels = 3);

    bool isStbiFlipVertically();
    void setStbiFlipVertically(bool value);

    bool memeqzero(const void* data, size_t length);

    constexpr int RESPONSE_CODE_FROM_CACHE = 1001;

    /**
     * @param url
     * @param useCache if db::requestCache should be used
     * @param sizeLimit stop download after sizeLimit bytes, default never stop
     * @param progressFunc int(void* clientp, long downloadTotal, long downloadNow, long uploadTotal, long uploadNow) // if return value is != 0, transfer stops
     * @return (responseCode, responseString)
     */
    std::pair<int, std::string> requestGET(const std::string& url, bool useCache = true, size_t sizeLimit = 0, int (*progressFunc)(void*, long, long, long, long) = nullptr);

    std::string readFileToString(const std::filesystem::path& path);

    template<class none = void>
    std::size_t combinedHash() {
        return 17;
    }

    template<typename T1, typename... Ts>
    std::size_t combinedHash(const T1& value1, const Ts&... values) {
        return combinedHash(values...) * 31 + robin_hood::hash<T1>()(value1);
    }
}
