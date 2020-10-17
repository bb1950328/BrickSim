// util.cpp
// Created by bb1950328 on 20.09.20.
//

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>
#include "util.h"

namespace util {
    std::string extend_home_dir(const std::string &input) {
        return extend_home_dir_path(input).string();
    }

    std::filesystem::path extend_home_dir_path(const std::string &input) {
        if (input[0] == '~' && (input[1] == '/' || input[1] == '\\')) {
            return std::filesystem::path(getenv(USER_ENV_VAR)) / std::filesystem::path(input.substr(2));
        } else {
            return std::filesystem::path(input);
        }
    }

    std::string trim(const std::string &input) {
        auto wsbefore = std::find_if_not(input.begin(), input.end(), [](int c) { return std::isspace(c); });
        auto wsafter = std::find_if_not(input.rbegin(), input.rend(), [](int c) { return std::isspace(c); }).base();
        return (wsafter <= wsbefore ? std::string() : std::string(wsbefore, wsafter));
    }

    std::string pathjoin(const std::list<std::string> &parts) {
        auto result = std::string();
        for (const auto &part : parts) {
            result.append(part);
            if (result.back() != PATH_SEPARATOR) {
                result.push_back(PATH_SEPARATOR);
            }
        }
        result.pop_back();//remove last separator
        std::replace(result.begin(), result.end(), PATH_SEPARATOR_FOREIGN, PATH_SEPARATOR);
        return result;
    }

    std::string as_lower(const std::string &string) {
        auto result = std::string();
        for (const auto &ch: string) {
            result.push_back(std::tolower(ch));
        }
        return result;
    }

    bool ends_with(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    bool starts_with(std::string const &fullString, std::string const &start) {
        return fullString.rfind(start, 0) == 0;
    }

    void cout_mat4(glm::mat4 mat) {/*
        std::cout << "⌈" << glm::to_string(mat[0]) << "⌉\n";
        std::cout << "|" << glm::to_string(mat[1]) << "|\n";
        std::cout << "|" << glm::to_string(mat[2]) << "|\n";
        std::cout << "⌊" << glm::to_string(mat[3]) << "⌋\n";*/
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[0][0], mat[0][1], mat[0][2] ,mat[0][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[1][0], mat[1][1], mat[1][2] ,mat[1][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[2][0], mat[2][1], mat[2][2] ,mat[2][3]);
        printf("%8.4f, %8.4f, %8.4f, %8.4f\n", mat[3][0], mat[3][1], mat[3][2] ,mat[3][3]);
    }

    void replaceAll(std::string &str, const std::string &from, const std::string &to) {
        //https://stackoverflow.com/a/3418285/8733066
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    std::string replaceChar(const std::string &str, char from, char to) {
        std::string result(str);
        if (from != to) {
            for (char &i : result) {
                if (i == from) {
                    i = to;
                }
            }
        }
        return result;
    }

    float biggest_value(glm::vec2 vector) {
        return std::max(vector.x, vector.y);
    }

    float biggest_value(glm::vec3 vector) {
        return std::max(std::max(vector.x, vector.y), vector.z);
    }

    float biggest_value(glm::vec4 vector) {
        return std::max(std::max(vector.x, vector.y), std::max(vector.z, vector.w));
    }

    unsigned long gcd(unsigned long a, unsigned long b) {
        //from https://www.geeksforgeeks.org/steins-algorithm-for-finding-gcd/
        if (a == 0)
            return b;
        if (b == 0)
            return a;

        unsigned long k;
        for (k = 0; (a | b) != 0 == 0; ++k) {
            a >>= 1u;
            b >>= 1u;
        }

        while ((a > 1) == 0) {
            a >>= 1u;
        }

        do {
            while ((b > 1) == 0) {
                b >>= 1u;
            }

            if (a > b) {
                std::swap(a, b);
            }

            b = (b - a);
        } while (b != 0);

        return a << k;
    }

    unsigned long lcm(unsigned long a, unsigned long b) {
        return a / gcd(a, b) * b;//https://stackoverflow.com/a/3154503/8733066
    }

    Fraction::Fraction(long a, long b) : a(a), b(b) {
        checkBnot0();
        simplify();
    }

    Fraction::Fraction(const Fraction &copyFrom) : a(copyFrom.a), b(copyFrom.b) {
    }

    void Fraction::checkBnot0() const {
        if (0 == b) {
            throw std::invalid_argument("b must not be 0");
        }
    }

    void Fraction::simplify() {
        auto gcd_ = gcd(std::abs(a), std::abs(b));
        a /= gcd_;
        b /= gcd_;
    }

    Fraction Fraction::operator+(const Fraction &other) const {
        return Fraction(a * other.b + other.a * b, b * other.b);
    }

    Fraction Fraction::operator-(const Fraction &other) const {
        return Fraction(a * other.b - other.a * b, b * other.b);
    }

    Fraction Fraction::operator*(const Fraction &other) const {
        return Fraction(a * other.a, b * other.b);
    }

    Fraction Fraction::operator/(const Fraction &other) const {
        return Fraction(a * other.b, b * other.a);
    }

    Fraction Fraction::operator+=(const Fraction &other) {
        auto tmpA = a * other.b + other.a * b;
        auto tmpB = b * other.b;
        a = tmpA;
        b = tmpB;
        return Fraction(*this);
    }

    Fraction Fraction::operator-=(const Fraction &other) {
        auto tmpA = a * other.b - other.a * b;
        auto tmpB = b * other.b;
        a = tmpA;
        b = tmpB;
        return Fraction(*this);
    }

    Fraction Fraction::operator*=(const Fraction &other) {
        a = a * other.a;
        b = b * other.b;
        return Fraction(*this);
    }

    Fraction Fraction::operator/=(const Fraction &other) {
        a = a * other.b;
        b = b * other.a;
        return Fraction(*this);
    }

    Fraction Fraction::operator+(long other) const {
        return Fraction(a + other * b, b);
    }

    Fraction Fraction::operator-(long other) const {
        return Fraction(a - other * b, b);
    }

    Fraction Fraction::operator*(long other) const {
        return Fraction(a * other, b);
    }

    Fraction Fraction::operator/(long other) const {
        return Fraction(a, b * other);
    }

    Fraction Fraction::operator+=(long other) {
        a = a + other * b;
        return Fraction(*this);
    }

    Fraction Fraction::operator-=(long other) {
        a = a - other * b;
        return Fraction(*this);
    }

    Fraction Fraction::operator*=(long other) {
        a = a * other;
        return Fraction(*this);
    }

    Fraction Fraction::operator/=(long other) {
        b = b * other;
        return Fraction(*this);
    }

    bool Fraction::operator==(const Fraction &other) const {
        return a == other.a && b == other.b;
    }

    bool Fraction::operator!=(const Fraction &other) const {
        return a != other.a || b != other.b;
    }

    bool Fraction::operator>(const Fraction &other) const {
        return a * other.b > other.a * b;
    }

    bool Fraction::operator<(const Fraction &other) const {
        return a * other.b < other.a * b;
    }

    bool Fraction::operator>=(const Fraction &other) const {
        return (a == other.a && b == other.b) || a * other.b > other.a * b;
    }

    bool Fraction::operator<=(const Fraction &other) const {
        return (a == other.a && b == other.b) || a * other.b < other.a * b;
    }

    std::ostream &operator<<(std::ostream &os, const Fraction &fraction) {
        os << "(" << fraction.a << " / " << fraction.b << ")";
        return os;
    }

    std::string Fraction::to_string() const {
        return std::string("(") + std::to_string(a) + " / " + std::to_string(b) + ")";
    }

    std::string Fraction::to_multiline_string() const {
        auto strA = std::to_string(a);
        auto strB = std::to_string(b);
        return strA + "\n" + std::string(std::max(strA.size(), strB.size()), '-') + "\n" + strB;
    }
}