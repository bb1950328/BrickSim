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

    bool isUvInsideImage(const glm::vec2& uv);

    // texture/image functions
    std::string translateBrickLinkColorNameToLDraw(std::string colorName);
    std::string translateLDrawColorNameToBricklink(std::string colorName);
    bool writeImage(const char* path, unsigned char* pixels, int width, int height, int channels = 3);

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

    /**
     * @param url
     * @param targetFile where the file should be saved
     * @param progressFunc int(void* clientp, long downloadTotal, long downloadNow, long uploadTotal, long uploadNow) // if return value is != 0, transfer stops
     * @return (responseCode, responseString)
     */
    std::pair<int, std::string> downloadFile(const std::string& url, const std::filesystem::path targetFile, int (*progressFunc)(void*, long, long, long, long) = nullptr);

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
