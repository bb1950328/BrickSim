#include "glm/gtx/transform.hpp"
#include <iostream>
#include <fstream>
#include "catch2/catch.hpp"
#include "../helpers/util.h"
#include "../helpers/platform_detection.h"

TEST_CASE("util::startsWith") {
    REQUIRE(util::startsWith("helloWorld", "hell"));
    REQUIRE(util::startsWith("abc", "abc"));
    REQUIRE(util::startsWith("a", "a"));
    REQUIRE_FALSE(util::startsWith("abc", "bc"));
    REQUIRE_FALSE(util::startsWith("abc", "abcd"));
    REQUIRE_FALSE(util::startsWith("abc", "cba"));

    REQUIRE(util::startsWith(std::string("qwertzuiop"), "qwer"));
}

TEST_CASE("util::endsWith") {
    REQUIRE(util::endsWith("helloWorld", "World"));
    REQUIRE(util::endsWith("abc", "abc"));
    REQUIRE(util::endsWith("a", "a"));
    REQUIRE_FALSE(util::endsWith("abc", "de"));
    REQUIRE_FALSE(util::endsWith("abc", "ab"));
    REQUIRE_FALSE(util::endsWith("abc", "aabc"));
}

TEST_CASE("util::asLower") {
    std::string expected = "qwertzuiopasdfghjklyxcvbnm";
    REQUIRE(expected == util::asLower("qwertzuiopasdfghjklyxcvbnm"));
    REQUIRE(expected == util::asLower("qWeRtZuIoPaSdFgHjKlYxCvBnM"));
    REQUIRE(expected == util::asLower("QWERTZUIOPASDFGHJKLYXCVBNM"));
    REQUIRE(std::string("1234567890+*ç%&/()=") == util::asLower("1234567890+*ç%&/()="));
}

TEST_CASE("util::toLowerInPlace") {
    std::string expected = "qwertzuiopasdfghjklyxcvbnm";
    std::string actual = "qwertzuiopasdfghjklyxcvbnm";
    util::toLowerInPlace(actual.data());
    REQUIRE(expected == actual);
    actual = "qWeRtZuIoPaSdFgHjKlYxCvBnM";
    util::toLowerInPlace(actual.data());
    REQUIRE(expected == actual);
    actual = "QWERTZUIOPASDFGHJKLYXCVBNM";
    util::toLowerInPlace(actual.data());
    REQUIRE(expected == actual);
}

TEST_CASE("util::asUpper") {
    std::string expected = "QWERTZUIOPASDFGHJKLYXCVBNM";
    REQUIRE(expected == util::asUpper("qwertzuiopasdfghjklyxcvbnm"));
    REQUIRE(expected == util::asUpper("qWeRtZuIoPaSdFgHjKlYxCvBnM"));
    REQUIRE(expected == util::asUpper("QWERTZUIOPASDFGHJKLYXCVBNM"));
    REQUIRE(std::string("1234567890+*ç%&/()=") == util::asUpper("1234567890+*ç%&/()="));
}

TEST_CASE("util::toUpperInPlace") {
    std::string expected = "QWERTZUIOPASDFGHJKLYXCVBNM";
    std::string actual = "QWERTZUIOPASDFGHJKLYXCVBNM";
    util::toUpperInPlace(actual.data());
    REQUIRE(expected == actual);
    actual = "qWeRtZuIoPaSdFgHjKlYxCvBnM";
    util::toUpperInPlace(actual.data());
    REQUIRE(expected == actual);
    actual = "qwertzuiopasdfghjklyxcvbnm";
    util::toUpperInPlace(actual.data());
    REQUIRE(expected == actual);
}

TEST_CASE("util::extendHomeDir and util::replaceHomeDir") {
    REQUIRE(util::extendHomeDir("~/abc") != "~/abc");
    REQUIRE(util::extendHomeDir("/abc/def") == "/abc/def");
    REQUIRE(util::extendHomeDir("/abc~def") == "/abc~def");
    REQUIRE(util::extendHomeDir("/abc~") == "/abc~");

    const char* expected = detected_platform::windows ? "~\\abc" : "~/abc";
    REQUIRE(util::replaceHomeDir(util::extendHomeDir("~\\abc")) == expected);
}

TEST_CASE("util::trim") {
    REQUIRE(util::trim("abc") == "abc");
    REQUIRE(util::trim(" abc") == "abc");
    REQUIRE(util::trim("\tabc") == "abc");
    REQUIRE(util::trim("\nabc") == "abc");
    REQUIRE(util::trim("\n abc") == "abc");
    REQUIRE(util::trim("\n \tabc") == "abc");
    REQUIRE(util::trim("a bc") == "a bc");
    REQUIRE(util::trim(" a bc") == "a bc");
    REQUIRE(util::trim(" a bc ") == "a bc");
    REQUIRE(util::trim(" a b c ") == "a b c");
    REQUIRE(util::trim(" a\tbc ") == "a\tbc");
}

TEST_CASE("util::replaceAll") {
    std::string str = "aaXXcc";
    SECTION("normal usage") {
        util::replaceAll(str, "XX", "bb");
        REQUIRE(str == "aabbcc");
    };
    SECTION("from not found") {
        util::replaceAll(str, "YY", "bb");
        REQUIRE(str == "aaXXcc");
    };
    SECTION("from at the beginning") {
        util::replaceAll(str, "aa", "zz");
        REQUIRE(str == "zzXXcc");
    };
    SECTION("from at the end") {
        util::replaceAll(str, "cc", "ZZ");
        REQUIRE(str == "aaXXZZ");
    };
    SECTION("from larger than to") {
        util::replaceAll(str, "XX", "Z");
        REQUIRE(str == "aaZcc");
    };
    SECTION("from smaller than to") {
        util::replaceAll(str, "XX", "ZZZ");
        REQUIRE(str == "aaZZZcc");
    };
    SECTION("multiple replacements") {
        util::replaceAll(str, "a", "Y");
        REQUIRE(str == "YYXXcc");
    };
    SECTION("multiple replacements, from smaller than to") {
        util::replaceAll(str, "a", "YZ");
        REQUIRE(str == "YZYZXXcc");
    };
    SECTION("str empty") {
        str = "";
        util::replaceAll(str, "a", "b");
        REQUIRE(str == "");
    };
    SECTION("from empty") {
        util::replaceAll(str, "", "a");
        REQUIRE(str == "aaXXcc");
    };
    SECTION("to empty") {
        util::replaceAll(str, "XX", "");
        REQUIRE(str == "aacc");
    }
}

TEST_CASE("util::replaceChar") {
    REQUIRE(util::replaceChar("aBc", 'B', 'b') == "abc");
    REQUIRE(util::replaceChar("abc", 'X', 'x') == "abc");
    REQUIRE(util::replaceChar("abc", 'a', 'A') == "Abc");
    REQUIRE(util::replaceChar("abcabc", 'a', 'A') == "AbcAbc");
    REQUIRE(util::replaceChar("", 'a', 'A').empty());
}

TEST_CASE("util::biggestValue") {
    REQUIRE(util::biggestValue({1000, 1}) == 1000);
    REQUIRE(util::biggestValue({1, 1000}) == 1000);

    REQUIRE(util::biggestValue({1000, 1, 2}) == 1000);
    REQUIRE(util::biggestValue({1, 1000, 2}) == 1000);
    REQUIRE(util::biggestValue({1, 2, 1000}) == 1000);

    REQUIRE(util::biggestValue({1000, 1, 2, 3}) == 1000);
    REQUIRE(util::biggestValue({1, 1000, 2, 3}) == 1000);
    REQUIRE(util::biggestValue({1, 2, 1000, 3}) == 1000);
    REQUIRE(util::biggestValue({1, 2, 3, 1000}) == 1000);
}

TEST_CASE("util::doesTransformationInverseWindingOrder") {
    REQUIRE_FALSE(util::doesTransformationInverseWindingOrder(glm::mat4(1.0f)));
    REQUIRE_FALSE(util::doesTransformationInverseWindingOrder(glm::translate(glm::mat4(1.0f), {1, 2, 3})));
    REQUIRE_FALSE(util::doesTransformationInverseWindingOrder(glm::rotate(glm::mat4(1.0f), 1.0f, {1, 2, 3})));
    REQUIRE_FALSE(util::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {1, 2, 3})));
    REQUIRE(util::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, 2, 3})));
    REQUIRE_FALSE(util::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, -2, 3})));
    REQUIRE(util::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, -2, -3})));
}

TEST_CASE("util::vectorSum") {
    REQUIRE(util::vectorSum({1.0f, 2.0f}) == Approx(3.0f));
    REQUIRE(util::vectorSum({1.0f, 2.0f, 3.0f}) == Approx(6.0f));
    REQUIRE(util::vectorSum({1.0f, 2.0f, 3.0f, 4.0f}) == Approx(10.0f));
}

TEST_CASE("util::formatBytesValue") {
    REQUIRE(util::formatBytesValue(1) == "1B");
    REQUIRE(util::formatBytesValue(10) == "10B");
    REQUIRE(util::formatBytesValue(100) == "100B");
    REQUIRE(util::formatBytesValue(1023) == "1023B");
    REQUIRE(util::formatBytesValue(1024) == "1.00KB");
    REQUIRE(util::formatBytesValue(1025) == "1.00KB");
    REQUIRE(util::formatBytesValue(10456) == "10.2KB");
    REQUIRE(util::formatBytesValue(104567) == "102KB");
    REQUIRE(util::formatBytesValue(1045678) == "1021KB");
    REQUIRE(util::formatBytesValue(10456789) == "9.97MB");

    REQUIRE(util::formatBytesValue(1ull << 10) == "1.00KB");
    REQUIRE(util::formatBytesValue(1ull << 20) == "1.00MB");
    REQUIRE(util::formatBytesValue(1ull << 30) == "1.00GB");
    REQUIRE(util::formatBytesValue(1ull << 40) == "1.00TB");
    REQUIRE(util::formatBytesValue(1ull << 50) == "1024TB");
    REQUIRE(util::formatBytesValue(1ull << 51) == "2048TB");
}

template<auto val>
using value_wrapper = std::integral_constant<decltype(val), val>;

TEMPLATE_TEST_CASE("util::memeqzero", "", value_wrapper<5>, value_wrapper<50>, value_wrapper<100>) {
    uint8_t buf1[TestType::value] = {0};
    REQUIRE(util::memeqzero(buf1, TestType::value));

    SECTION("modify first byte") {
        buf1[0] = 1;
        REQUIRE_FALSE(util::memeqzero(buf1, TestType::value));
    };
    SECTION("modify middle byte") {
        buf1[2] = 1;
        REQUIRE_FALSE(util::memeqzero(buf1, TestType::value));
    };
    SECTION("modify last byte") {
        buf1[TestType::value - 1] = 1;
        REQUIRE_FALSE(util::memeqzero(buf1, TestType::value));
    }
}

TEST_CASE("util::readFileToString") {
    auto filename = std::filesystem::temp_directory_path() / "BrickSimUnitTest.txt";

    std::ofstream out(filename);
    out << "Hello World!";
    out.close();

    REQUIRE(util::readFileToString(filename) == "Hello World!");

    std::filesystem::remove(filename);
}

TEST_CASE("util::minForEachComponent") {
    REQUIRE(util::minForEachComponent({1, 1}, {2, 2}) == glm::vec2(1, 1));
    REQUIRE(util::minForEachComponent({1, 2}, {2, 1}) == glm::vec2(1, 1));

    REQUIRE(util::minForEachComponent({1, 2, 3}, {3, 2, 1}) == glm::vec3(1, 2, 1));

    REQUIRE(util::minForEachComponent({1, 2, 3, 4}, {5, 4, 3, 2}) == glm::vec4(1, 2, 3, 2));
}

//these are macros because when using lambdas the catch error output is less readable
#define REQUIRE_EQUALS_ALPHANUM(a, b) REQUIRE(util::equalsAlphanum(a, b)); REQUIRE(util::equalsAlphanum(b, a));
#define REQUIRE_FALSE_EQUALS_ALPHANUM(a, b) REQUIRE_FALSE(util::equalsAlphanum(a, b)); REQUIRE_FALSE(util::equalsAlphanum(b, a));

TEST_CASE("util::equalsAlphanum") {
    REQUIRE_EQUALS_ALPHANUM("", "");
    REQUIRE_EQUALS_ALPHANUM("", "");
    REQUIRE_EQUALS_ALPHANUM("a", "a");
    REQUIRE_EQUALS_ALPHANUM("abc", "abc");
    REQUIRE_EQUALS_ALPHANUM("a$bc", "abc");
    REQUIRE_EQUALS_ALPHANUM("a$$bc", "abc");
    REQUIRE_EQUALS_ALPHANUM("a$b$c", "abc");
    REQUIRE_EQUALS_ALPHANUM("$abc", "abc");
    REQUIRE_EQUALS_ALPHANUM("abc$", "abc");

    REQUIRE_EQUALS_ALPHANUM("abc$", "abc$");
    REQUIRE_EQUALS_ALPHANUM("$abc$", "!abc$");
    REQUIRE_EQUALS_ALPHANUM("$ab&c$", "!a*bc$");

    REQUIRE_FALSE_EQUALS_ALPHANUM("abc", "cba");
    REQUIRE_FALSE_EQUALS_ALPHANUM("a", "");
    REQUIRE_FALSE_EQUALS_ALPHANUM("a", "b");
    REQUIRE_FALSE_EQUALS_ALPHANUM("a", "bc");
    REQUIRE_FALSE_EQUALS_ALPHANUM("ab", "bc");

    REQUIRE_FALSE_EQUALS_ALPHANUM("*ab", "*bc");
    REQUIRE_FALSE_EQUALS_ALPHANUM("*ab*", "*b&c");
}

TEST_CASE("util::withoutBasePath") {
    std::filesystem::path abc("abc");
    std::filesystem::path def("def");
    std::filesystem::path ghi("ghi");
    REQUIRE(util::withoutBasePath(abc / def / ghi, abc / def) == ghi);
    REQUIRE(util::withoutBasePath(abc / def / ghi, abc) == def / ghi);
    REQUIRE(util::withoutBasePath(abc / def / abc, abc) == def / abc);
    REQUIRE(util::withoutBasePath(abc / abc / def / abc, abc) == abc / def / abc);
    REQUIRE(util::withoutBasePath(abc, abc) == "");
    REQUIRE(util::withoutBasePath(abc, def) == abc);
    REQUIRE(util::withoutBasePath(abc / def, def) == abc / def);
}

TEST_CASE("util::containsIgnoreCase") {
    REQUIRE(util::containsIgnoreCase("abcdefg", "cde"));
    REQUIRE(util::containsIgnoreCase("abcdefg", "CdE"));
    REQUIRE(util::containsIgnoreCase("aBcDeFg", "CdE"));
    REQUIRE(util::containsIgnoreCase("aBcDeFg", "C"));
    REQUIRE(util::containsIgnoreCase("aBcDeFg", "CD"));
    REQUIRE(util::containsIgnoreCase("aBcDeFg", "ab"));
    REQUIRE(util::containsIgnoreCase("aBcDeFg", "fg"));
    REQUIRE(util::containsIgnoreCase("aBcDeFg", "aBcDeFg"));
    REQUIRE_FALSE(util::containsIgnoreCase("aBcDeFg", "df"));
    REQUIRE_FALSE(util::containsIgnoreCase("aBcDeFg", "aBcDeFgHiJ"));
}

TEST_CASE("util::calculateDistanceOfPointToLine") {
    REQUIRE(util::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {1.0f, 1.0f}, {0.0f, 1.0f}) == Approx(std::sqrt(2) / 2));
    REQUIRE(util::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {1.0f, 0.0f}, {1.0f, 1.0f}) == Approx(1));
    REQUIRE(util::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {0.0f, 1.0f}, {1.0f, 1.0f}) == Approx(1));
}

TEST_CASE("util::normalProjectionOnLine0") {
    auto result = util::normalProjectionOnLine({0, 0}, {2, 0}, {1, 1});
    REQUIRE(result.distancePointToLine == Approx(1));
    REQUIRE(result.lineLength == Approx(2));
    REQUIRE(result.nearestPointOnLine.x == Approx(1));
    REQUIRE(result.nearestPointOnLine.y == Approx(0));
    REQUIRE(result.projection.x == Approx(1));
    REQUIRE(result.projection.y == Approx(0));
    REQUIRE(result.projectionLength == Approx(1));
}

TEST_CASE("util::normalProjectionOnLine1") {
    auto result = util::normalProjectionOnLine({1, 1}, {3, 3}, {1, 3});
    REQUIRE(result.distancePointToLine == Approx(sqrt(2)));
    REQUIRE(result.lineLength == Approx(2 * sqrt(2)));
    REQUIRE(result.nearestPointOnLine.x == Approx(2));
    REQUIRE(result.nearestPointOnLine.y == Approx(2));
    REQUIRE(result.projection.x == Approx(1));
    REQUIRE(result.projection.y == Approx(1));
    REQUIRE(result.projectionLength == Approx(sqrt(2)));
}

TEST_CASE("util::normalProjectionOnLine2") {
    auto result = util::normalProjectionOnLine({10, 20}, {14, 22}, {11, 23});
    REQUIRE(result.distancePointToLine == Approx(sqrt(2*2+1*1)));
    REQUIRE(result.lineLength == Approx(sqrt(4*4+2*2)));
    REQUIRE(result.nearestPointOnLine.x == Approx(12));
    REQUIRE(result.nearestPointOnLine.y == Approx(21));
    REQUIRE(result.projection.x == Approx(2));
    REQUIRE(result.projection.y == Approx(1));
    REQUIRE(result.projectionLength == Approx(sqrt(2*2+1*1)));
}