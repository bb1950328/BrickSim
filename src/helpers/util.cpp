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

    DecomposedTransformation decomposeTransformationToStruct(const glm::mat4& transformation) {
        DecomposedTransformation result{};
        glm::decompose(transformation, result.scale, result.orientation, result.translation, result.skew, result.perspective);
        return result;
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
}