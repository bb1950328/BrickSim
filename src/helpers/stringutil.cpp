#include "stringutil.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#ifdef __SSE2__

    #include <cstring>
    #include <glm/gtx/matrix_decompose.hpp>
    #include <immintrin.h>
    #include <iomanip>
    #include <sstream>

#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
    #include <arm_neon.h>
#endif

namespace bricksim::stringutil {
    std::string trim(const std::string& input) {
        auto wsbefore = std::find_if_not(input.begin(), input.end(), [](int c) { return c > 0 && c <= 0xff && std::isspace(c); });
        auto wsafter = std::find_if_not(input.rbegin(), input.rend(), [](int c) { return c > 0 && c <= 0xff && std::isspace(c); }).base();
        return (wsafter <= wsbefore ? std::string() : std::string(wsbefore, wsafter));
    }

    std::string_view trim(const std::string_view input) {
        if (input.empty()) {
            return input;
        }
        std::size_t wsbefore = 0;
        std::size_t wsafter = input.size();
        while (wsbefore < input.size() && input[wsbefore] > 0 && input[wsbefore] <= 0xff && std::isspace(input[wsbefore])) {
            ++wsbefore;
        }
        do {
            --wsafter;
        } while (wsafter > 0 && input[wsafter] > 0 && input[wsafter] <= 0xff && std::isspace(input[wsafter]));

        return (wsafter < wsbefore ? std::string_view() : input.substr(wsbefore, wsafter - wsbefore + 1));
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
        asLower(string, string, std::strlen(string));
    }

    void toUpperInPlace(char* string) {
        asUpper(string, string, std::strlen(string));
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

    bool containsIgnoreCase(const std::string& full, const std::string& sub) {
        return std::search(
                       full.begin(), full.end(),
                       sub.begin(), sub.end(),
                       [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); })
               != full.end();
    }

    std::vector<std::string_view> splitByChar(std::string_view string, char delimiter) {
        std::size_t start = 0;
        std::size_t end = 0;
        std::vector<std::string_view> words;
        while (start < string.size()) {
            end = string.find(delimiter, start);
            if (end == std::string_view::npos) {
                end = string.size();
            }
            if (start < end) {
                words.push_back(string.substr(start, end - start));
            }
            start = end + 1;
        }
        return words;
    }
    bool charEqualsIgnoreCase(char a, char b) {
        return std::tolower(a) == std::tolower(b);
    }
    bool stringEqualsIgnoreCase(std::string_view a, std::string_view b) {
        return std::equal(a.begin(), a.end(),
                          b.begin(), b.end(),
                          [](char a, char b) {
                              return tolower(a) == tolower(b);
                          });
    }
}