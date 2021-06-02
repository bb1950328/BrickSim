#include "glm/gtx/transform.hpp"
#include <iostream>
#include <fstream>
#include <glm/gtc/epsilon.hpp>
#include "catch2/catch.hpp"
#include "../../helpers/util.h"
#include "../../helpers/platform_detection.h"
#include "../testing_tools.h"

TEST_CASE("util::startsWith") {
    CHECK(util::startsWith("helloWorld", "hell"));
    CHECK(util::startsWith("abc", "abc"));
    CHECK(util::startsWith("a", "a"));
    CHECK_FALSE(util::startsWith("abc", "bc"));
    CHECK_FALSE(util::startsWith("abc", "abcd"));
    CHECK_FALSE(util::startsWith("abc", "cba"));

    CHECK(util::startsWith(std::string("qwertzuiop"), "qwer"));
}

TEST_CASE("util::endsWith") {
    CHECK(util::endsWith("helloWorld", "World"));
    CHECK(util::endsWith("abc", "abc"));
    CHECK(util::endsWith("a", "a"));
    CHECK_FALSE(util::endsWith("abc", "de"));
    CHECK_FALSE(util::endsWith("abc", "ab"));
    CHECK_FALSE(util::endsWith("abc", "aabc"));
}

TEST_CASE("util::asLower") {
    std::string expected = "qwertzuiopasdfghjklyxcvbnm";
    CHECK(expected == util::asLower("qwertzuiopasdfghjklyxcvbnm"));
    CHECK(expected == util::asLower("qWeRtZuIoPaSdFgHjKlYxCvBnM"));
    CHECK(expected == util::asLower("QWERTZUIOPASDFGHJKLYXCVBNM"));
    CHECK(std::string("1234567890+*ç%&/()=") == util::asLower("1234567890+*ç%&/()="));
}

TEST_CASE("util::toLowerInPlace") {
    std::string expected = "qwertzuiopasdfghjklyxcvbnm";
    std::string actual = "qwertzuiopasdfghjklyxcvbnm";
    util::toLowerInPlace(actual.data());
    CHECK(expected == actual);
    actual = "qWeRtZuIoPaSdFgHjKlYxCvBnM";
    util::toLowerInPlace(actual.data());
    CHECK(expected == actual);
    actual = "QWERTZUIOPASDFGHJKLYXCVBNM";
    util::toLowerInPlace(actual.data());
    CHECK(expected == actual);
}

TEST_CASE("util::asUpper") {
    std::string expected = "QWERTZUIOPASDFGHJKLYXCVBNM";
    CHECK(expected == util::asUpper("qwertzuiopasdfghjklyxcvbnm"));
    CHECK(expected == util::asUpper("qWeRtZuIoPaSdFgHjKlYxCvBnM"));
    CHECK(expected == util::asUpper("QWERTZUIOPASDFGHJKLYXCVBNM"));
    CHECK(std::string("1234567890+*ç%&/()=") == util::asUpper("1234567890+*ç%&/()="));
}

TEST_CASE("util::toUpperInPlace") {
    std::string expected = "QWERTZUIOPASDFGHJKLYXCVBNM";
    std::string actual = "QWERTZUIOPASDFGHJKLYXCVBNM";
    util::toUpperInPlace(actual.data());
    CHECK(expected == actual);
    actual = "qWeRtZuIoPaSdFgHjKlYxCvBnM";
    util::toUpperInPlace(actual.data());
    CHECK(expected == actual);
    actual = "qwertzuiopasdfghjklyxcvbnm";
    util::toUpperInPlace(actual.data());
    CHECK(expected == actual);
}

TEST_CASE("util::extendHomeDir and util::replaceHomeDir") {
    CHECK(util::extendHomeDir("~/abc") != "~/abc");
    CHECK(util::extendHomeDir("/abc/def") == "/abc/def");
    CHECK(util::extendHomeDir("/abc~def") == "/abc~def");
    CHECK(util::extendHomeDir("/abc~") == "/abc~");

    const char *expected = detected_platform::windows ? "~\\abc" : "~/abc";
    CHECK(util::replaceHomeDir(util::extendHomeDir("~\\abc")) == expected);
}

TEST_CASE("util::trim") {
    CHECK(util::trim("abc") == "abc");
    CHECK(util::trim(" abc") == "abc");
    CHECK(util::trim("\tabc") == "abc");
    CHECK(util::trim("\nabc") == "abc");
    CHECK(util::trim("\n abc") == "abc");
    CHECK(util::trim("\n \tabc") == "abc");
    CHECK(util::trim("a bc") == "a bc");
    CHECK(util::trim(" a bc") == "a bc");
    CHECK(util::trim(" a bc ") == "a bc");
    CHECK(util::trim(" a b c ") == "a b c");
    CHECK(util::trim(" a\tbc ") == "a\tbc");
}

TEST_CASE("util::replaceAll") {
    std::string str = "aaXXcc";
    SECTION("normal usage") {
        util::replaceAll(str, "XX", "bb");
        CHECK(str == "aabbcc");
    };
    SECTION("from not found") {
        util::replaceAll(str, "YY", "bb");
        CHECK(str == "aaXXcc");
    };
    SECTION("from at the beginning") {
        util::replaceAll(str, "aa", "zz");
        CHECK(str == "zzXXcc");
    };
    SECTION("from at the end") {
        util::replaceAll(str, "cc", "ZZ");
        CHECK(str == "aaXXZZ");
    };
    SECTION("from larger than to") {
        util::replaceAll(str, "XX", "Z");
        CHECK(str == "aaZcc");
    };
    SECTION("from smaller than to") {
        util::replaceAll(str, "XX", "ZZZ");
        CHECK(str == "aaZZZcc");
    };
    SECTION("multiple replacements") {
        util::replaceAll(str, "a", "Y");
        CHECK(str == "YYXXcc");
    };
    SECTION("multiple replacements, from smaller than to") {
        util::replaceAll(str, "a", "YZ");
        CHECK(str == "YZYZXXcc");
    };
    SECTION("str empty") {
        str = "";
        util::replaceAll(str, "a", "b");
        CHECK(str == "");
    };
    SECTION("from empty") {
        util::replaceAll(str, "", "a");
        CHECK(str == "aaXXcc");
    };
    SECTION("to empty") {
        util::replaceAll(str, "XX", "");
        CHECK(str == "aacc");
    }
}

TEST_CASE("util::replaceChar") {
    CHECK(util::replaceChar("aBc", 'B', 'b') == "abc");
    CHECK(util::replaceChar("abc", 'X', 'x') == "abc");
    CHECK(util::replaceChar("abc", 'a', 'A') == "Abc");
    CHECK(util::replaceChar("abcabc", 'a', 'A') == "AbcAbc");
    CHECK(util::replaceChar("", 'a', 'A').empty());
}

TEST_CASE("util::biggestValue") {
    CHECK(util::biggestValue({1000, 1}) == 1000);
    CHECK(util::biggestValue({1, 1000}) == 1000);

    CHECK(util::biggestValue({1000, 1, 2}) == 1000);
    CHECK(util::biggestValue({1, 1000, 2}) == 1000);
    CHECK(util::biggestValue({1, 2, 1000}) == 1000);

    CHECK(util::biggestValue({1000, 1, 2, 3}) == 1000);
    CHECK(util::biggestValue({1, 1000, 2, 3}) == 1000);
    CHECK(util::biggestValue({1, 2, 1000, 3}) == 1000);
    CHECK(util::biggestValue({1, 2, 3, 1000}) == 1000);
}

TEST_CASE("util::doesTransformationInverseWindingOrder") {
    CHECK_FALSE(util::doesTransformationInverseWindingOrder(glm::mat4(1.0f)));
    CHECK_FALSE(util::doesTransformationInverseWindingOrder(glm::translate(glm::mat4(1.0f), {1, 2, 3})));
    CHECK_FALSE(util::doesTransformationInverseWindingOrder(glm::rotate(glm::mat4(1.0f), 1.0f, {1, 2, 3})));
    CHECK_FALSE(util::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {1, 2, 3})));
    CHECK(util::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, 2, 3})));
    CHECK_FALSE(util::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, -2, 3})));
    CHECK(util::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, -2, -3})));
}

TEST_CASE("util::vectorSum") {
    CHECK(util::vectorSum({1.0f, 2.0f}) == Approx(3.0f));
    CHECK(util::vectorSum({1.0f, 2.0f, 3.0f}) == Approx(6.0f));
    CHECK(util::vectorSum({1.0f, 2.0f, 3.0f, 4.0f}) == Approx(10.0f));
}

TEST_CASE("util::formatBytesValue") {
    CHECK(util::formatBytesValue(1) == "1B");
    CHECK(util::formatBytesValue(10) == "10B");
    CHECK(util::formatBytesValue(100) == "100B");
    CHECK(util::formatBytesValue(1023) == "1023B");
    CHECK(util::formatBytesValue(1024) == "1.00KB");
    CHECK(util::formatBytesValue(1025) == "1.00KB");
    CHECK(util::formatBytesValue(10456) == "10.2KB");
    CHECK(util::formatBytesValue(104567) == "102KB");
    CHECK(util::formatBytesValue(1045678) == "1021KB");
    CHECK(util::formatBytesValue(10456789) == "9.97MB");

    CHECK(util::formatBytesValue(1ull << 10) == "1.00KB");
    CHECK(util::formatBytesValue(1ull << 20) == "1.00MB");
    CHECK(util::formatBytesValue(1ull << 30) == "1.00GB");
    CHECK(util::formatBytesValue(1ull << 40) == "1.00TB");
    CHECK(util::formatBytesValue(1ull << 50) == "1024TB");
    CHECK(util::formatBytesValue(1ull << 51) == "2048TB");
}

template<auto val>
using value_wrapper = std::integral_constant<decltype(val), val>;

TEMPLATE_TEST_CASE("util::memeqzero", "", value_wrapper<5>, value_wrapper<50>, value_wrapper<100>) {
    uint8_t buf1[TestType::value] = {0};
    REQUIRE(util::memeqzero(buf1, TestType::value));

    SECTION("modify first byte") {
        buf1[0] = 1;
        CHECK_FALSE(util::memeqzero(buf1, TestType::value));
    };
    SECTION("modify middle byte") {
        buf1[2] = 1;
        CHECK_FALSE(util::memeqzero(buf1, TestType::value));
    };
    SECTION("modify last byte") {
        buf1[TestType::value - 1] = 1;
        CHECK_FALSE(util::memeqzero(buf1, TestType::value));
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
    CHECK(util::minForEachComponent({1, 1}, {2, 2}) == glm::vec2(1, 1));
    CHECK(util::minForEachComponent({1, 2}, {2, 1}) == glm::vec2(1, 1));

    CHECK(util::minForEachComponent({1, 2, 3}, {3, 2, 1}) == glm::vec3(1, 2, 1));

    CHECK(util::minForEachComponent({1, 2, 3, 4}, {5, 4, 3, 2}) == glm::vec4(1, 2, 3, 2));
}

//these are macros because when using lambdas the catch error output is less readable
#define CHECK_EQUALS_ALPHANUM(a, b) CHECK(util::equalsAlphanum(a, b)); CHECK(util::equalsAlphanum(b, a));
#define CHECK_FALSE_EQUALS_ALPHANUM(a, b) CHECK_FALSE(util::equalsAlphanum(a, b)); CHECK_FALSE(util::equalsAlphanum(b, a));

TEST_CASE("util::equalsAlphanum") {
    CHECK_EQUALS_ALPHANUM("", "");
    CHECK_EQUALS_ALPHANUM("", "");
    CHECK_EQUALS_ALPHANUM("a", "a");
    CHECK_EQUALS_ALPHANUM("abc", "abc");
    CHECK_EQUALS_ALPHANUM("a$bc", "abc");
    CHECK_EQUALS_ALPHANUM("a$$bc", "abc");
    CHECK_EQUALS_ALPHANUM("a$b$c", "abc");
    CHECK_EQUALS_ALPHANUM("$abc", "abc");
    CHECK_EQUALS_ALPHANUM("abc$", "abc");

    CHECK_EQUALS_ALPHANUM("abc$", "abc$");
    CHECK_EQUALS_ALPHANUM("$abc$", "!abc$");
    CHECK_EQUALS_ALPHANUM("$ab&c$", "!a*bc$");

    CHECK_FALSE_EQUALS_ALPHANUM("abc", "cba");
    CHECK_FALSE_EQUALS_ALPHANUM("a", "");
    CHECK_FALSE_EQUALS_ALPHANUM("a", "b");
    CHECK_FALSE_EQUALS_ALPHANUM("a", "bc");
    CHECK_FALSE_EQUALS_ALPHANUM("ab", "bc");

    CHECK_FALSE_EQUALS_ALPHANUM("*ab", "*bc");
    CHECK_FALSE_EQUALS_ALPHANUM("*ab*", "*b&c");
}

TEST_CASE("util::withoutBasePath") {
    std::filesystem::path abc("abc");
    std::filesystem::path def("def");
    std::filesystem::path ghi("ghi");
    CHECK(util::withoutBasePath(abc / def / ghi, abc / def) == ghi);
    CHECK(util::withoutBasePath(abc / def / ghi, abc) == def / ghi);
    CHECK(util::withoutBasePath(abc / def / abc, abc) == def / abc);
    CHECK(util::withoutBasePath(abc / abc / def / abc, abc) == abc / def / abc);
    CHECK(util::withoutBasePath(abc, abc) == "");
    CHECK(util::withoutBasePath(abc, def) == abc);
    CHECK(util::withoutBasePath(abc / def, def) == abc / def);
}

TEST_CASE("util::containsIgnoreCase") {
    CHECK(util::containsIgnoreCase("abcdefg", "cde"));
    CHECK(util::containsIgnoreCase("abcdefg", "CdE"));
    CHECK(util::containsIgnoreCase("aBcDeFg", "CdE"));
    CHECK(util::containsIgnoreCase("aBcDeFg", "C"));
    CHECK(util::containsIgnoreCase("aBcDeFg", "CD"));
    CHECK(util::containsIgnoreCase("aBcDeFg", "ab"));
    CHECK(util::containsIgnoreCase("aBcDeFg", "fg"));
    CHECK(util::containsIgnoreCase("aBcDeFg", "aBcDeFg"));
    CHECK_FALSE(util::containsIgnoreCase("aBcDeFg", "df"));
    CHECK_FALSE(util::containsIgnoreCase("aBcDeFg", "aBcDeFgHiJ"));
}

TEST_CASE("util::calculateDistanceOfPointToLine") {
    CHECK(util::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {1.0f, 1.0f}, {0.0f, 1.0f}) == Approx(std::sqrt(2) / 2));
    CHECK(util::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {1.0f, 0.0f}, {1.0f, 1.0f}) == Approx(1));
    CHECK(util::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {0.0f, 1.0f}, {1.0f, 1.0f}) == Approx(1));
}

TEST_CASE("util::normalProjectionOnLine0") {
    auto result = util::normalProjectionOnLineClamped({0, 0}, {2, 0}, {1, 1});
    CHECK(result.distancePointToLine == Approx(1));
    CHECK(result.lineLength == Approx(2));
    CHECK(result.nearestPointOnLine == ApproxVec(glm::vec2(1, 0)));
    CHECK(result.projection == ApproxVec(glm::vec2(1, 0)));
    CHECK(result.projectionLength == Approx(1));
}

TEST_CASE("util::normalProjectionOnLine1") {
    auto result = util::normalProjectionOnLineClamped({1, 1}, {3, 3}, {1, 3});
    CHECK(result.distancePointToLine == Approx(sqrt(2)));
    CHECK(result.lineLength == Approx(2 * sqrt(2)));
    CHECK(result.nearestPointOnLine == ApproxVec(glm::vec2(2, 2)));
    CHECK(result.projection == ApproxVec(glm::vec2(1, 1)));
    CHECK(result.projectionLength == Approx(sqrt(2)));
}

TEST_CASE("util::normalProjectionOnLine2") {
    auto result = util::normalProjectionOnLineClamped({10, 20}, {14, 22}, {11, 23});
    CHECK(result.distancePointToLine == Approx(sqrt(2 * 2 + 1 * 1)));
    CHECK(result.lineLength == Approx(sqrt(4 * 4 + 2 * 2)));
    CHECK(result.nearestPointOnLine == ApproxVec(glm::vec2(12, 21)));
    CHECK(result.projection == ApproxVec(glm::vec2(2, 1)));
    CHECK(result.projectionLength == Approx(sqrt(2 * 2 + 1 * 1)));
}

void consistencyCheck(const glm::vec3 &startA, const glm::vec3 &dirA, const glm::vec3 &startB, const glm::vec3 &dirB, const util::ClosestLineBetweenTwoRaysResult &result) {
    CHECK(glm::length(result.pointOnA - result.pointOnB) == result.distanceBetweenPoints);
    CHECK(startA + dirA * result.distanceToPointA == ApproxVec(result.pointOnA));
    CHECK(startB + dirB * result.distanceToPointB == ApproxVec(result.pointOnB));
}

TEST_CASE("util::closestLineBetweenTwoRays0") {
    // https://keisan.casio.com/exec/system/1223531414
    const glm::vec3 startA = {-1, 2, 0};
    const glm::vec3 dirA = {2, 3, 1};
    const glm::vec3 startB = {3, -4, 1};
    const glm::vec3 dirB = {1, 2, 1};
    auto result = util::closestLineBetweenTwoRays({startA, dirA}, {startB, dirB});
    consistencyCheck(startA, dirA, startB, dirB, result);
    CHECK(result.distanceBetweenPoints == Approx(6.3508529610859));
    CHECK(result.pointOnA == ApproxVec(glm::vec3(5, 11, 3)));
    CHECK(result.pointOnB == ApproxVec(glm::vec3(26 / 3.f, 22 / 3.f, 20 / 3.f)));
}

TEST_CASE("util::closestLineBetweenTwoRays1") {
    const glm::vec3 startA = {1, 2, 3};
    const glm::vec3 dirA = {2, 3, 4};
    const glm::vec3 startB = {3, 2, 1};
    const glm::vec3 dirB = {4, 3, 2};
    auto result = util::closestLineBetweenTwoRays({startA, dirA}, {startB, dirB});
    consistencyCheck(startA, dirA, startB, dirB, result);
    CHECK(result.distanceBetweenPoints == Approx(0.0f));
    CHECK(result.pointOnA == ApproxVec(glm::vec3(-1, -1, -1)));
    CHECK(result.pointOnB == ApproxVec(glm::vec3(-1, -1, -1)));
}

TEST_CASE("util::closestLineBetweenTwoRays2") {
    const glm::vec3 startA = {1, 2, 3};
    const glm::vec3 dirA = {2, 3, 4};
    const glm::vec3 startB = {3, 2, 1};
    const glm::vec3 dirB = dirA;
    auto result = util::closestLineBetweenTwoRays({startA, dirA}, {startB, dirB});
    consistencyCheck(startA, dirA, startB, dirB, result);
    CHECK(result.distanceBetweenPoints == Approx(2.7291529568841f));
    CHECK(result.pointOnA == ApproxVec(startA));
    CHECK(result.pointOnB == ApproxVec(glm::vec3(3.27586198, 2.41379309, 1.5517242)));
}

TEST_CASE("util::closestLineBetweenTwoRays3") {
    const glm::vec3 startA = {9999, 9999, 50};
    const glm::vec3 dirA = {2, 3, 0};
    const glm::vec3 startB = {9999, 9999, 150};
    const glm::vec3 dirB = {3, 2, 0};
    auto result = util::closestLineBetweenTwoRays({startA, dirA}, {startB, dirB});
    consistencyCheck(startA, dirA, startB, dirB, result);
    CHECK(result.distanceBetweenPoints == Approx(100));
    CHECK(result.pointOnA == ApproxVec(glm::vec3(9999, 9999, 50)));
    CHECK(result.pointOnB == ApproxVec(glm::vec3(9999, 9999, 150)));
}