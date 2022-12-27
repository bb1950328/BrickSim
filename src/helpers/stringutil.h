#pragma once
#include <string>
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
    void replaceAll(std::string& str, const std::string& from, const std::string& to);
    std::string replaceChar(const std::string& str, char from, char to);
    std::string formatBytesValue(uint64_t bytes);
    bool containsIgnoreCase(const std::string& full, const std::string& sub);
    bool charEqualsIgnoreCase(char a, char b);
    bool stringEqualsIgnoreCase(std::string_view a, std::string_view b);
    bool equalsAlphanum(const std::string& a, const std::string& b);
    std::vector<std::string_view> splitByChar(std::string_view string, char delimiter);
}
