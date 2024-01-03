#pragma once

#include "../types.h"
#include "../constant_data/constants.h"
#include "ray.h"
#include <filesystem>
#include <glm/glm.hpp>
#include <optional>
#include <span>
#include <string>
#include <bit>
//#include <CImg.h>

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
    const std::string ALPHANUM_CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    // path functions
    std::string extendHomeDir(const std::string& input);
    //std::string replaceSpecialPaths(const std::string& input);
    std::filesystem::path replaceSpecialPaths(const std::string& input);
    uomap_t<std::string, std::filesystem::path> getSpecialPaths();
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
    glm::vec<L, T, Q> cwiseMin(const glm::vec<L, T, Q>& a, const glm::vec<L, T, Q>& b) {
        glm::vec<L, T, Q> res;
        for (int i = 0; i < L; ++i) {
            res[i] = std::min(a[i], b[i]);
        }
        return res;
    }

    template<glm::length_t L, typename T, glm::qualifier Q>
    glm::vec<L, T, Q> cwiseMax(const glm::vec<L, T, Q>& a, const glm::vec<L, T, Q>& b) {
        glm::vec<L, T, Q> res;
        for (int i = 0; i < L; ++i) {
            res[i] = std::max(a[i], b[i]);
        }
        return res;
    }

    template<typename V, typename M>
        requires std::is_arithmetic_v<V> && std::is_arithmetic_v<M>
    V roundToNearestMultiple(V value, M multiple) {
        return std::round(value / static_cast<V>(multiple)) * static_cast<V>(multiple);
    }

    template<glm::length_t L, typename T, glm::qualifier Q, typename M>
        requires std::is_arithmetic_v<M>
    glm::vec<L, T, Q> roundToNearestMultiple(const glm::vec<L, T, Q>& value, M multiple) {
        glm::vec<L, T, Q> result;
        for (int i = 0; i < L; ++i) {
            result[i] = roundToNearestMultiple(value[i], multiple);
        }
        return result;
    }

    template<glm::length_t L, typename TV, typename TM, glm::qualifier Q>
    glm::vec<L, TV, Q> roundToNearestMultiple(const glm::vec<L, TV, Q>& value, const glm::vec<L, TM, Q>& multiple) {
        glm::vec<L, TV, Q> result;
        for (int i = 0; i < L; ++i) {
            result[i] = roundToNearestMultiple<TV, TM>(value[i], multiple[i]);
        }
        return result;
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

    struct RawImage {
        int width;
        int height;
        int channels;
        std::vector<unsigned char> data;

        unsigned char* getPixel(uint16_t x, uint16_t y);
    };

    RawImage readImage(std::span<const uint8_t> fileData);

    bool writeImage(const char* path, const RawImage& image);

    bool writeImage(const char* path, const unsigned char* pixels, int width, int height, int channels = 3);
    /*bool writeImage(const char* path, cimg_library::CImg<unsigned char> img);*/

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
    std::pair<int, std::string> downloadFile(const std::string& url, std::filesystem::path targetFile, int (*progressFunc)(void*, long, long, long, long) = nullptr);

    std::string readFileToString(const std::filesystem::path& path);

    template<class none = void>
    std::size_t combinedHash() {
        return 17;
    }

    template<typename T1, typename... Ts>
    std::size_t combinedHash(const T1& value1, const Ts&... values) {
        return combinedHash(values...) * 31 + hash<T1>{}(value1);
    }

    constexpr auto& getOrDefault(const auto& map, const auto& key, const auto& defaultValue) {
        const auto it = map.find(key);
        return it == map.cend() ? defaultValue : it->second;
    }

    std::string randomAlphanumString(uint64_t length);

    class NoCopy {
    public:
        NoCopy() = default;
        NoCopy(const NoCopy&) = delete;
        NoCopy& operator=(const NoCopy&) = delete;
    };

    class NoMove {
    public:
        NoMove() = default;
        NoMove(NoMove&&) = delete;
        NoMove& operator=(NoMove&&) = delete;
    };

    class NoCopyNoMove : public NoCopy, public NoMove {
    public:
        NoCopyNoMove() = default;
    };

    template<typename Iterator>
        requires std::is_floating_point_v<typename std::iterator_traits<Iterator>::value_type>
    constexpr bool floatRangeEpsilonEqual(Iterator aBegin, Iterator aEnd, Iterator bBegin, Iterator bEnd, float epsilon) {
        return std::distance(aBegin, aEnd) == std::distance(bBegin, bEnd)
               && std::equal(aBegin, aEnd, bBegin, [epsilon](const auto& a, const auto& b) { return std::fabs(a - b) < epsilon; });
    }

    template<glm::length_t L, typename T>
    constexpr bool vecVecEpsilonEqual(const std::vector<glm::vec<L, T>>& a, const std::vector<glm::vec<L, T>>& b, float epsilon) {
        return a.size() == b.size()
               && std::equal(a.cbegin(), a.cend(), b.cbegin(), [epsilon](const auto& va, const auto& vb) { return glm::all(glm::epsilonEqual(va, vb, epsilon)); });
    }

    template<typename ForwardIterator>
    ForwardIterator removeDuplicates(ForwardIterator first, ForwardIterator last) {
        //https://stackoverflow.com/a/49863804/8733066
        auto newLast = first;

        for (auto current = first; current != last; ++current) {
            if (std::find(first, newLast, *current) == newLast) {
                if (newLast != current)
                    *newLast = *current;
                ++newLast;
            }
        }

        return newLast;
    }

    std::string escapeFilename(const std::string& original);

    void setThreadName(const char* threadName);

    template<typename T>
    struct ScopeVarBackup {
        T backup;
        T& ref;

        ScopeVarBackup(T& ref) :
            backup(ref), ref(ref) {}

        ScopeVarBackup(T& ref, T newValue) :
            ScopeVarBackup(ref) {
            ref = newValue;
        }

        ~ScopeVarBackup() {
            ref = backup;
        }
    };

    template<class T1, class T2, class Predicate = std::less<T1>>
    struct compare_pair_first {
        bool operator()(const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) {
            return Predicate{}(left.first, right.first);
        }
    };

    template<class T1, class T2, class Predicate = std::less<T2>>
    struct compare_pair_second {
        bool operator()(const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) {
            return Predicate{}(left.second, right.second);
        }
    };

    struct UtfType {
        std::uint8_t bits;
        std::uint8_t bomLength;
        std::endian endian;
    };

    UtfType determineUtfTypeFromBom(const std::string_view text);
}
