#pragma once
#include "../gui/icons.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include <algorithm>
#include <chrono>
#include <glm/glm.hpp>
#include <spdlog/fmt/fmt.h>
#include <string>
#include <utf8.h>
#include <vector>

namespace bricksim::stringutil {
    std::string trim(const std::string& input);
    std::string_view trim(std::string_view input);
    void asLower(const char* input, char* output, size_t length);
    std::string asLower(const std::string& string);
    std::string asLower(std::string_view string);
    void asUpper(const char* input, char* output, size_t length);
    std::string asUpper(const std::string& string);
    std::string asUpper(std::string_view string);
    void toLowerInPlace(char* string);
    void toUpperInPlace(char* string);
    void replaceAll(std::string& str, std::string_view from, std::string_view to);
    std::string replaceChar(const std::string& str, char from, char to);
    std::string formatBytesValue(uint64_t bytes);
    bool containsIgnoreCase(std::string_view full, std::string_view sub);
    bool charEqualsIgnoreCase(char a, char b);
    bool stringEqualsIgnoreCase(std::string_view a, std::string_view b);
    bool equalsAlphanum(std::string_view a, std::string_view b);
    std::vector<std::string_view> splitByChar(std::string_view string, char delimiter);
    std::string repeat(const std::string& str, unsigned int times);

    std::string escapeXml(const std::string& str);

    template<glm::length_t L, typename T>
    std::string formatGLM(const glm::vec<L, T>& vec);

    template<typename T>
    std::string formatGLM(const glm::vec<2, T>& vec) {
        return fmt::format("vec2({:g}, {:g})", vec.x, vec.y);
    }

    template<typename T>
    std::string formatGLM(const glm::vec<3, T>& vec) {
        return fmt::format("vec3({:g}, {:g}, {:g})", vec.x, vec.y, vec.z);
    }

    template<typename T>
    std::string formatGLM(const glm::vec<4, T>& vec) {
        return fmt::format("vec4({:g}, {:g}, {:g}, {:g})", vec.x, vec.y, vec.z, vec.w);
    }

    template<glm::length_t L, typename T>
    std::string formatGLM(const glm::mat<L, L, T>& mat);

    template<typename T>
    std::string formatGLM(const glm::mat<2, 2, T>& mat) {
        return fmt::format("mat2(({:g}, {:g}), ({:g}, {:g}))",
                           mat[0][0], mat[0][1],
                           mat[1][0], mat[1][1]);
    }

    template<typename T>
    std::string formatGLM(const glm::mat<3, 3, T>& mat) {
        return fmt::format("mat3(({:g}, {:g}, {:g}), ({:g}, {:g}, {:g}), ({:g}, {:g}, {:g}))",
                           mat[0][0], mat[0][1], mat[0][2],
                           mat[1][0], mat[1][1], mat[1][2],
                           mat[2][0], mat[2][1], mat[2][2]);
    }

    template<typename T>
    std::string formatGLM(const glm::mat<4, 4, T>& mat) {
        return fmt::format("mat4(({:g}, {:g}, {:g}, {:g}), ({:g}, {:g}, {:g}, {:g}), ({:g}, {:g}, {:g}, {:g}), ({:g}, {:g}, {:g}, {:g}))",
                           mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                           mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                           mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                           mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    }

    std::string removeIcons(std::string_view withIcons);

    /**
     * @return the index of the first occurrence of " that is >= start and not preceded by a backslash
     */
    std::size_t findClosingQuote(std::string_view str, std::size_t start = 0);

    std::chrono::year_month_day parseYYYY_MM_DD(std::string_view str);
}
