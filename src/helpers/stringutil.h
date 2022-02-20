#pragma once
#include <string>

namespace bricksim::stringutil {
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
}