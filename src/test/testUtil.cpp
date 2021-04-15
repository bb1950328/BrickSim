#include "catch2/catch.hpp"
#include "../helpers/util.h"

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

    REQUIRE(util::replaceHomeDir(util::extendHomeDir("~/abc")) == "~/abc");
}

TEST_CASE("test") {
    REQUIRE(false);
}