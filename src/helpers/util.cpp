#include "util.h"
#include "../config.h"
#include "glm/gtc/matrix_transform.hpp"
#include "platform_detection.h"
#include <array>
#include <cstring>
#include <curl/curl.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/normal.hpp>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stb_image.h>
#include <stb_image_write.h>

#ifdef BRICKSIM_PLATFORM_WINDOWS
    #include <windows.h>
#endif

#ifdef __SSE2__

    #include <glm/gtx/matrix_decompose.hpp>
    #include <immintrin.h>

#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
    #include <arm_neon.h>
#endif

namespace bricksim::util {
    namespace {

        bool isStbiVerticalFlipEnabled = false;
    }

    std::string extendHomeDir(const std::string& input) {
        return extendHomeDirPath(input).string();
    }

    std::string replaceHomeDir(const std::string& input) {
        const std::string homeDir = getenv(USER_ENV_VAR);
        if (startsWith(input, homeDir)) {
            return '~' + input.substr(homeDir.size());
        }
        return input;
    }

    std::filesystem::path extendHomeDirPath(const std::string& input) {
        if (input[0] == '~' && (input[1] == '/' || input[1] == '\\')) {
            return std::filesystem::path(getenv(USER_ENV_VAR)) / std::filesystem::path(input.substr(2));
        } else if (input[0] == '~' && input.size() == 1) {
            return std::filesystem::path(getenv(USER_ENV_VAR));
        } else {
            return std::filesystem::path(input);
        }
    }

    std::string trim(const std::string& input) {
        auto wsbefore = std::find_if_not(input.begin(), input.end(), [](int c) { return std::isspace(c); });
        auto wsafter = std::find_if_not(input.rbegin(), input.rend(), [](int c) { return std::isspace(c); }).base();
        return (wsafter <= wsbefore ? std::string() : std::string(wsbefore, wsafter));
    }

    void asLower(const char* input, char* output, size_t length) {
#ifdef BRICKSIM_USE_OPTIMIZED_VARIANTS
    #ifdef __SSE2__
        const __m128i asciiA = _mm_set1_epi8('A' - 1);
        const __m128i asciiZ = _mm_set1_epi8('Z' + 1);
        const __m128i diff = _mm_set1_epi8('a' - 'A');
        while (length >= 16) {
            __m128i inp = _mm_loadu_si128((__m128i*)input);
            /* >= 'A': 0xff, < 'A': 0x00 */
            __m128i greaterEqualA = _mm_cmpgt_epi8(inp, asciiA);
            /* <= 'Z': 0xff, > 'Z': 0x00 */
            __m128i lessEqualZ = _mm_cmplt_epi8(inp, asciiZ);
            /* 'Z' >= x >= 'A': 0xFF, else 0x00 */
            __m128i mask = _mm_and_si128(greaterEqualA, lessEqualZ);
            /* 'Z' >= x >= 'A': 'a' - 'A', else 0x00 */
            __m128i toAdd = _mm_and_si128(mask, diff);
            /* add to change to lowercase */
            __m128i added = _mm_add_epi8(inp, toAdd);
            _mm_storeu_si128((__m128i*)output, added);
            length -= 16;
            input += 16;
            output += 16;
        }
    #elif defined(__ARM_NEON) || defined(__ARM_NEON__)
        const uint8x16_t asciiA = vdupq_n_u8('A');
        const uint8x16_t asciiZ = vdupq_n_u8('Z' + 1);
        const uint8x16_t diff = vdupq_n_u8('a' - 'A');
        while (length >= 16) {
            uint8x16_t inp = vld1q_u8((uint8_t*)input);
            uint8x16_t greaterThanA = vcgtq_u8(inp, asciiA);
            uint8x16_t lessEqualZ = vcltq_u8(inp, asciiZ);
            uint8x16_t mask = vandq_u8(greaterThanA, lessEqualZ);
            uint8x16_t toAdd = vandq_u8(mask, diff);
            uint8x16_t added = vaddq_u8(inp, toAdd);
            vst1q_u8((uint8_t*)output, added);
            length -= 16;
            input += 16;
            output += 16;
        }
    #endif
#endif
        while (length-- > 0) {
            *output = tolower(*input);
            ++input;
            ++output;
        }
    }

    std::string asLower(const std::string& string) {
        std::string result;
        result.resize(string.length());
        asLower(string.c_str(), result.data(), string.length());
        return result;
    }

    void asUpper(const char* input, char* output, size_t length) {
#ifdef BRICKSIM_USE_OPTIMIZED_VARIANTS
    #ifdef __SSE2__
        const __m128i asciia = _mm_set1_epi8('a' - 1);
        const __m128i asciiz = _mm_set1_epi8('z' + 1);
        const __m128i diff = _mm_set1_epi8('a' - 'A');
        while (length >= 16) {
            __m128i inp = _mm_loadu_si128((__m128i*)input);
            /* > 'a': 0xff, < 'a': 0x00 */
            __m128i greaterThana = _mm_cmpgt_epi8(inp, asciia);
            /* <= 'z': 0xff, > 'z': 0x00 */
            __m128i lessEqualz = _mm_cmplt_epi8(inp, asciiz);
            /* 'z' >= x >= 'a': 0xFF, else 0x00 */
            __m128i mask = _mm_and_si128(greaterThana, lessEqualz);
            /* 'z' >= x >= 'a': 'a' - 'A', else 0x00 */
            __m128i toSub = _mm_and_si128(mask, diff);
            /* subtract to change to uppercase */
            __m128i added = _mm_sub_epi8(inp, toSub);
            _mm_storeu_si128((__m128i*)output, added);
            length -= 16;
            input += 16;
            output += 16;
        }
    #elif defined(__ARM_NEON) || defined(__ARM_NEON__)
        const uint8x16_t asciia = vdupq_n_u8('a' - 1);
        const uint8x16_t asciiz = vdupq_n_u8('z' + 1);
        const uint8x16_t diff = vdupq_n_u8('a' - 'A');
        while (length >= 16) {
            uint8x16_t inp = vld1q_u8((uint8_t*)input);
            uint8x16_t greaterThana = vcgtq_u8(inp, asciia);
            uint8x16_t lessEqualz = vcltq_u8(inp, asciiz);
            uint8x16_t mask = vandq_u8(greaterThana, lessEqualz);
            uint8x16_t toSub = vandq_u8(mask, diff);
            uint8x16_t added = vsubq_u8(inp, toSub);
            vst1q_u8((uint8_t*)output, added);
            length -= 16;
            input += 16;
            output += 16;
        }
    #endif
#endif
        while (length-- > 0) {
            *output = toupper(*input);
            ++input;
            ++output;
        }
    }

    std::string asUpper(const std::string& string) {
        std::string result;
        result.resize(string.length());
        asUpper(string.c_str(), result.data(), string.length());
        return result;
    }

    void toLowerInPlace(char* string) {
        asLower(string, string, strlen(string));
    }

    void toUpperInPlace(char* string) {
        asUpper(string, string, strlen(string));
    }

    bool endsWithInternal(const char* fullString, size_t fullStringSize, const char* ending, size_t endingSize) {
        if (endingSize > fullStringSize) {
            return false;
        }
        return strncmp(fullString + fullStringSize - endingSize, ending, endingSize) == 0;
    }

    bool endsWith(std::string const& fullString, std::string const& ending) {
        return endsWithInternal(fullString.c_str(), fullString.size(), ending.c_str(), ending.size());
    }

    bool endsWith(const char* fullString, const char* ending) {
        return endsWithInternal(fullString, strlen(fullString), ending, strlen(ending));
    }

    bool startsWith(std::string const& fullString, std::string const& start) {
        if (fullString.length() < start.length()) {
            return false;
        }
        return startsWith(fullString.c_str(), start.c_str());
    }

    bool startsWith(const char* fullString, const char* start) {
        do {
            if (*start != *fullString) {
                return false;
            }
            ++start;
            ++fullString;
        } while (*start && *fullString);
        if (*start == 0) {
            return true;
        } else {
            return *fullString != 0;
        }
    }

    void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        //https://stackoverflow.com/a/3418285/8733066
        //todo maybe this has optimization potential
        if (from.empty()) {
            return;
        }
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();// In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    std::string replaceChar(const std::string& str, char from, char to) {
        std::string result(str);
        if (from != to) {
            for (char& i: result) {
                if (i == from) {
                    i = to;
                }
            }
        }
        return result;
    }

    void openDefaultBrowser(const std::string& link) {
        spdlog::info("openDefaultBrowser(\"{}\")", link);
#ifdef BRICKSIM_PLATFORM_WINDOWS
        ShellExecute(nullptr, "open", link.c_str(), nullptr, nullptr, SW_SHOWNORMAL);//todo testing
#elif defined(BRICKSIM_PLATFORM_MACOS)
        std::string command = std::string("open ") + link;
#elif defined(BRICKSIM_PLATFORM_LINUX)
        std::string command = std::string("xdg-open ") + link;
#else
    #warning "openDefaultProwser not supported on this platform"
#endif
#if defined(BRICKSIM_PLATFORM_LINUX) || defined(BRICKSIM_PLATFORM_MACOS)
        int exitCode = system(command.c_str());
        if (exitCode != 0) {
            spdlog::warn("command \"{}\" exited with code {}", command, exitCode);
        }
#endif
    }

    glm::vec3 triangleCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
        return (p1 + p2 + p3) / 3.0f;
    }

    glm::vec3 quadrilateralCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4) {
        return (p1 + p2 + p3 + p4) / 4.0f;
    }

    bool doesTransformationInverseWindingOrder(const glm::mat4& transformation) {
        glm::vec3 vec1 = transformation[0];
        glm::vec3 vec2 = transformation[1];
        glm::vec3 vec3 = transformation[2];
        glm::vec3 cross = glm::cross(vec1, vec2);
        return glm::dot(cross, vec3) < 0.0f;
    }

    std::string formatBytesValue(uint64_t bytes) {
        double doubleBytes = bytes;
        static std::string bytePrefixes[] = {"B", "KB", "MB", "GB", "TB"};
        size_t prefixIndex = 0;
        while (doubleBytes >= 1024 && prefixIndex < std::size(bytePrefixes) - 1) {
            prefixIndex++;
            doubleBytes /= 1024;
        }
        std::stringstream resultStream;
        if (prefixIndex > 0) {
            resultStream << std::fixed << std::setprecision(std::max(0, 2 - (int)(std::log10(doubleBytes))));
        }
        resultStream << doubleBytes << bytePrefixes[prefixIndex];
        return resultStream.str();
    }

    bool memeqzero(const void* data, size_t length) {
        //from https://github.com/rustyrussell/ccan/blob/master/ccan/mem/mem.c#L92
        const auto* p = static_cast<const unsigned char*>(data);
        size_t len;

        /* Check first 16 bytes manually */
        for (len = 0; len < 16; len++) {
            if (!length) {
                return true;
            }
            if (*p) {
                return false;
            }
            p++;
            length--;
        }

        /* Now we know that's zero, memcmp with self. */
        return memcmp(data, p, length) == 0;
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

    bool equalsAlphanum(const std::string& a, const std::string& b) {
        auto itA = a.cbegin();
        auto itB = b.cbegin();
        while (itA != a.cend() || itB != b.cend()) {
            while (itA != a.cend() && !std::isalnum(*itA)) {
                ++itA;
            }
            while (itB != b.cend() && !std::isalnum(*itB)) {
                ++itB;
            }
            if (itA == a.cend() && itB == b.cend()) {
                return true;
            }
            if ((itA == a.cend()) != (itB == b.cend())) {
                return false;
            }
            if (itA != a.cend() && *itA != *itB) {
                return false;
            }
            ++itA;
            ++itB;
        }
        return itA == a.cend() && itB == b.cend();
    }

    std::filesystem::path withoutBasePath(const std::filesystem::path& path, const std::filesystem::path& basePath) {
        auto itPath = path.begin();
        auto itBase = basePath.begin();
        while (itPath != path.end() && itBase != basePath.end() && *itPath == *itBase) {
            ++itPath;
            ++itBase;
        }
        std::filesystem::path result;
        std::for_each(itPath, path.end(), [&result](auto part) {
            result /= part;
        });
        return result;
    }

    bool writeImage(const char* path, unsigned char* pixels, unsigned int width, unsigned int height, int channels) {
        auto path_lower = util::asLower(path);
        stbi_flip_vertically_on_write(true);
        if (util::endsWith(path_lower, ".png")) {
            return stbi_write_png(path, width, height, channels, pixels, width * channels) != 0;
        } else if (util::endsWith(path_lower, ".jpg") || util::endsWith(path, ".jpeg")) {
            const int quality = std::min(100, std::max(5, (int)config::get(config::JPG_SCREENSHOT_QUALITY)));
            return stbi_write_jpg(path, width, height, channels, pixels, quality) != 0;
        } else if (util::endsWith(path_lower, ".bmp")) {
            return stbi_write_bmp(path, width, height, channels, pixels) != 0;
        } else if (util::endsWith(path_lower, ".tga")) {
            return stbi_write_tga(path, width, height, channels, pixels) != 0;
        } else {
            return false;
        }
    }

    bool containsIgnoreCase(const std::string& full, const std::string& sub) {
        return std::search(
                       full.begin(), full.end(),
                       sub.begin(), sub.end(),
                       [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); })
               != full.end();
    }

    bool isStbiFlipVertically() {
        return isStbiVerticalFlipEnabled;
    }

    void setStbiFlipVertically(bool value) {
        isStbiVerticalFlipEnabled = value;
        stbi_set_flip_vertically_on_load(value ? 1 : 0);
    }

    size_t oldWriteFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    size_t oldWriteFunction4kb(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        if (data->size() > 4096) {
            return 0;
        }
        return size * nmemb;
    }

    std::pair<int, std::string> requestGET(const std::string& url, bool useCache, size_t sizeLimit, int (*progressFunc)(void*, long, long, long, long)) {
        if (useCache) {
            auto fromCache = db::requestCache::get(url);
            if (fromCache.has_value()) {
                spdlog::info("Request GET {} cache hit", url);
                return {RESPONSE_CODE_FROM_CACHE, fromCache.value()};
            }
        }
        spdlog::info("Request GET {} {}", url, useCache ? " cache miss" : " without cache");
        auto curl = curl_easy_init();
        if (!curl) {
            return {-1, ""};
        }
        char errorBuffer[CURL_ERROR_SIZE+1];
        errorBuffer[0] = '\0';

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.135 Safari/537.36 Edge/12.10136");//sorry microsoft ;)
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10'000L);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

        /*curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [sizeLimit](void *ptr, size_t size, size_t nmemb, std::string *data) -> size_t {
            data->append((char *) ptr, size * nmemb);
            if (sizeLimit > 0 && data->size()>sizeLimit) {
                return 0;
            }
            return size * nmemb;
        });*/
        //todo build something threadsafe which can pass sizeLimit to writeFunction
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (0 < sizeLimit && sizeLimit < 4096) ? oldWriteFunction4kb : oldWriteFunction);

        if (progressFunc != nullptr) {
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressFunc);
        } else {
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        }

        std::string response_string;
        std::string header_string;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        const CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            spdlog::error("cURL error on GET request to {}: {} | {}", url, magic_enum::enum_name(result), errorBuffer);
        }

        char* effectiveUrl;
        long response_code;
        double elapsed;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);

        if (useCache && 100 <= response_code && response_code < 400 && result==CURLE_OK && !response_string.empty()) {
            db::requestCache::put(url, response_string);
        }

        curl_easy_cleanup(curl);

        return {response_code, response_string};
    }

    std::string readFileToString(const std::filesystem::path& path) {
        FILE* f = fopen(path.string().c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long length = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::string buffer;
            buffer.resize(length);
            size_t bytesRead = fread(&buffer[0], 1, length, f);
            if (bytesRead != length) {
                spdlog::warn("reading file {}: {} bytes read, but reported size is {} bytes.", path.string(), bytesRead, length);
                buffer.resize(bytesRead);
            }
            fclose(f);
            return buffer;
        } else {
            spdlog::error("can't read file {}: ", path.string(), strerror(errno));
            throw std::invalid_argument(strerror(errno));
        }
    }

    float calculateDistanceOfPointToLine(const glm::vec2& line_start, const glm::vec2& line_end, const glm::vec2& point) {
        //https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
        float numerator = std::abs((line_end.x - line_start.x) * (line_start.y - point.y) - (line_start.x - point.x) * (line_end.y - line_start.y));
        float denominator = std::sqrt(std::pow(line_end.x - line_start.x, 2.0f) + std::pow(line_end.y - line_start.y, 2.0f));
        return numerator / denominator;
    }

    float calculateDistanceOfPointToLine(const glm::usvec2& line_start, const glm::usvec2& line_end, const glm::usvec2& point) {
        int numerator = std::abs((line_end.x - line_start.x) * (line_start.y - point.y) - (line_start.x - point.x) * (line_end.y - line_start.y));
        float denominator = std::sqrt(std::pow(line_end.x - line_start.x, 2.0f) + std::pow(line_end.y - line_start.y, 2.0f));
        return numerator / denominator;
    }

    NormalProjectionResult normalProjectionOnLineClamped(const glm::vec2& lineStart, const glm::vec2& lineEnd, const glm::vec2& point) {
        //https://stackoverflow.com/a/47366970/8733066
        //             point
        //             +
        //          /  |
        //       /     |
        // start-------⦝----- end
        //             ↑
        //         nearestPointOnLine
        //
        // projection is from start to nearestPointOnLine
        NormalProjectionResult result{};
        glm::vec2 line = lineEnd - lineStart;
        result.lineLength = glm::length(line);
        glm::vec2 lineUnit = line / result.lineLength;
        glm::vec2 startToPoint = point - lineStart;
        result.projectionLength = glm::dot(startToPoint, lineUnit);

        if (result.projectionLength > result.lineLength) {
            result.nearestPointOnLine = lineEnd;
            result.projectionLength = result.lineLength;
            result.projection = line;
        } else if (result.projectionLength < 0.0f) {
            result.nearestPointOnLine = lineStart;
            result.projectionLength = 0.0f;
            result.projection = {0, 0};
        } else {
            result.projection = lineUnit * result.projectionLength;
            result.nearestPointOnLine = lineStart + result.projection;
        }
        result.distancePointToLine = glm::length(point - result.nearestPointOnLine);

        return result;
    }

    void gaussianElimination(std::array<float, 12>& matrix) {
        constexpr auto cols = 4;
        constexpr auto rows = 3;
        for (int i = 0; i < cols - 1; i++) {
            for (int j = i; j < rows; j++) {
                if (matrix[i + j * cols] != 0) {
                    if (i != j) {
                        for (int k = i; k < cols; k++) {
                            std::swap(matrix[k + i * cols], matrix[k + j * cols]);
                        }
                    }

                    j = i;

                    for (int v = 0; v < rows; v++) {
                        if (v == j) {
                            continue;
                        } else {
                            float factor = matrix[i + v * cols] / matrix[i + j * cols];
                            matrix[i + v * cols] = 0;

                            for (int u = i + 1; u < cols; u++) {
                                matrix[u + v * cols] -= factor * matrix[u + j * cols];
                                matrix[u + j * cols] /= matrix[i + j * cols];
                            }
                            matrix[i + j * cols] = 1;
                        }
                    }
                    break;
                }
            }
        }
    }

    ClosestLineBetweenTwoRaysResult closestLineBetweenTwoRays(const Ray3& a, const Ray3& b) {
        const auto p1 = a.origin;
        const auto d1 = glm::normalize(a.direction);
        const auto p2 = b.origin;
        const auto d2 = glm::normalize(b.direction);

        if (glm::all(glm::epsilonEqual(d1, d2, 0.001f))) {
            //rays are parallel -> we can do a normal projection
            //there are infinite solutions in this case so we set pointOnA to startA
            glm::vec3 startToStart = p1 - p2;
            float x = (glm::dot(d2, startToStart) / glm::length2(d2));
            auto pointOnB = p2 + x * d2;
            return {
                    .pointOnA = p1,
                    .pointOnB = pointOnB,
                    .distanceToPointA = 0,
                    .distanceToPointB = x,
                    .distanceBetweenPoints = glm::length(p1 - pointOnB),
            };
        } else {
            //https://math.stackexchange.com/a/1702955
            const auto n = glm::cross(d1, d2);
            const auto n1 = glm::cross(d1, n);
            const auto n2 = glm::cross(d2, n);
            const auto factor1 = glm::dot((p2 - p1), n2) / glm::dot(d1, n2);
            const auto factor2 = glm::dot((p1 - p2), n1) / glm::dot(d2, n1);
            const auto c1 = p1 + factor1 * d1;
            const auto c2 = p2 + factor2 * d2;

            return {
                    .pointOnA = c1,
                    .pointOnB = c2,
                    .distanceToPointA = factor1,
                    .distanceToPointB = factor2,
                    .distanceBetweenPoints = glm::length(c1 - c2),
            };
        }
    }

    DecomposedTransformation decomposeTransformationToStruct(const glm::mat4& transformation) {
        DecomposedTransformation result{};
        glm::decompose(transformation, result.scale, result.orientation, result.translation, result.skew, result.perspective);
        return result;
    }

    std::optional<glm::vec3> rayPlaneIntersection(const Ray3& ray, const Ray3& planeNormal) {
        //https://math.stackexchange.com/a/2121529/945069
        const auto normalizedRayDir = glm::normalize(ray.direction);
        const auto normalizedPlaneDir = glm::normalize(planeNormal.direction);
        const float rayScale = glm::dot(normalizedPlaneDir, (planeNormal.origin - ray.origin)) / glm::dot(normalizedPlaneDir, normalizedRayDir);
        if (rayScale < 0) {
            return {};
        } else {
            return ray.origin + rayScale * normalizedRayDir;
        }
    }

    float getAngleBetweenThreePointsUnsigned(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        const auto ab = b - a;
        const auto bc = b - c;
        return std::acos(glm::dot(ab, bc) / (glm::length(ab) * glm::length(bc)));
    }

    float getAngleBetweenThreePointsSigned(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& planeNormal) {
        const auto ab = b - a;
        const auto cb = b - c;
        return std::atan2(glm::dot(glm::cross(ab, cb), glm::normalize(planeNormal)), glm::dot(ab, cb));
    }

    glm::quat quaternionRotationFromOneVectorToAnother(const glm::vec3& v1, const glm::vec3& v2) {
        //https://stackoverflow.com/a/11741520/8733066 second solution
        const auto v1n = glm::normalize(v1);
        const auto v2n = glm::normalize(v2);
        float k_cos_theta = glm::dot(v1n, v2n);
        float k = glm::sqrt(glm::length2(v1n) * glm::length2(v2n));
        if (k_cos_theta / k == -1) {
            // 180° rotation around any vector
            return {0, glm::normalize(getAnyPerpendicularVector(v1n))};
        }
        return glm::normalize(glm::quat(k_cos_theta + k, glm::cross(v1n, v2n)));
    }

    glm::vec3 getAnyPerpendicularVector(const glm::vec3& v) {
        return {1.f, 1.f, (-v.x - v.y) / v.z};
    }

    float getDistanceBetweenPointAndPlane(const Ray3& planeNormal, const glm::vec3& point) {
        //basically a perpendicular projection on the normal line
        return glm::dot(point - planeNormal.origin, glm::normalize(planeNormal.direction));
    }

    std::optional<glm::vec3> linePlaneIntersection(const glm::vec3& lineP0, const glm::vec3& lineP1, const Ray3& planeNormal) {
        auto u = lineP1 - lineP0;
        const auto dot = glm::dot(planeNormal.direction, u);
        if (glm::abs(dot) > 1e-6f) {
            const auto w = lineP0 - planeNormal.origin;
            const auto fac = -glm::dot(planeNormal.direction, w) / dot;
            u *= fac;
            return lineP0 + u;
        } else {
            return {};
        }
    }

    bool isPointInTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& point) {
        const auto a = p1 - point;
        const auto b = p2 - point;
        const auto c = p3 - point;

        const auto u = glm::cross(b, c);
        const auto v = glm::cross(c, a);
        const auto w = glm::cross(a, b);

        return glm::dot(u, v) >= 0.f && glm::dot(u, w) >= 0.f;
    }

    bool sutherlandHogmanInside(const glm::vec2& p, const glm::vec2& p1, const glm::vec2& p2) {
        return (p2.y - p1.y) * p.x + (p1.x - p2.x) * p.y + (p2.x * p1.y - p1.x * p2.y) < 0;
    }

    glm::vec2 sutherlandHogmanIntersection(const glm::vec2& cp1, const glm::vec2& cp2, const glm::vec2& s, const glm::vec2& e) {
        const glm::vec2 dc = {cp1.x - cp2.x, cp1.y - cp2.y};
        const glm::vec2 dp = {s.x - e.x, s.y - e.y};

        const float n1 = cp1.x * cp2.y - cp1.y * cp2.x;
        const float n2 = s.x * e.y - s.y * e.x;
        const float n3 = 1.0f / (dc.x * dp.y - dc.y * dp.x);

        return {(n1 * dp.x - n2 * dc.x) * n3, (n1 * dp.y - n2 * dc.y) * n3};
    }

    std::vector<glm::vec2> sutherlandHogmanPolygonClipping(const std::vector<glm::vec2>& subjectPolygon, const std::vector<glm::vec2>& clipPolygon) {
        glm::vec2 cp1, cp2, s, e;
        std::vector<glm::vec2> inputPolygon;
        std::vector<glm::vec2> outputPolygon = subjectPolygon;
        cp1 = clipPolygon[clipPolygon.size() - 1];
        for (const auto& clipVertex: clipPolygon) {
            cp2 = clipVertex;
            inputPolygon = outputPolygon;
            outputPolygon.clear();
            s = inputPolygon[inputPolygon.size() - 1];

            for (const auto& subjectVertex: inputPolygon) {
                e = subjectVertex;
                const bool sInside = sutherlandHogmanInside(s, cp1, cp2);
                const bool eInside = sutherlandHogmanInside(e, cp1, cp2);
                if (eInside) {
                    if (!sInside) {
                        outputPolygon.push_back(sutherlandHogmanIntersection(cp1, cp2, s, e));
                    }
                    outputPolygon.push_back(e);
                } else if (sInside) {
                    outputPolygon.push_back(sutherlandHogmanIntersection(cp1, cp2, s, e));
                }
                s = e;
            }
            cp1 = cp2;
        }
        return outputPolygon;
    }

    bool is2dPolygonClockwise(const std::vector<glm::vec2>& polygon) {
        float signedArea = getSignedPolygonArea(polygon);
        return signedArea < 0;
    }

    float getSignedPolygonArea(const std::vector<glm::vec2>& polygon) {
        float signedArea = 0;
        for (size_t i1 = 0, i2 = 1; i1 < polygon.size(); ++i1, i2 = (i1 + 1) % polygon.size()) {
            signedArea += (polygon[i1].x * polygon[i2].y - polygon[i2].x * polygon[i1].y);
        }
        signedArea *= 0.5;
        return signedArea;
    }

    glm::mat4 DecomposedTransformation::orientationAsMat4() const {
        return glm::toMat4(orientation);
    }

    glm::mat4 DecomposedTransformation::translationAsMat4() const {
        return glm::translate(glm::mat4(1.0f), translation);
    }

    glm::mat4 DecomposedTransformation::scaleAsMat4() const {
        return glm::scale(glm::mat4(1.0f), scale);
    }

    Plane3dTo2dConverter::Plane3dTo2dConverter(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) :
        origin(a), xDir(glm::normalize(b - a)) {
        planeNormal = glm::triangleNormal(a, b, c);
        yDir = glm::rotate(glm::mat4(1.f), static_cast<float>(M_PI_2), planeNormal) * glm::vec4(xDir, 0.f);
    }

    glm::vec2 Plane3dTo2dConverter::convert3dTo2d(const glm::vec3& pointOnPlane) {
        const auto& r_P = pointOnPlane;
        const auto& n = planeNormal;
        const auto& r_O = origin;
        const auto& e_1 = xDir;
        const auto& e_2 = yDir;

        const auto s = glm::dot(n, r_P - r_O);
        const auto t_1 = glm::dot(e_1, r_P - r_O);
        const auto t_2 = glm::dot(e_2, r_P - r_O);
        return {t_1, t_2};
    }

    glm::vec3 Plane3dTo2dConverter::convert2dTo3d(const glm::vec2& coord) {
        return origin + coord.x * xDir + coord.y * yDir;
    }
}