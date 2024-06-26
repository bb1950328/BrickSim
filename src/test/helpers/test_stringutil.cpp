#include "../../helpers/stringutil.h"
#include "catch2/catch_test_macros.hpp"
#include <glm/gtc/epsilon.hpp>

//#include <string_view>

using namespace std::literals;

namespace bricksim {
    TEST_CASE("stringutil::asLower") {
        std::string expected = "qwertzuiopasdfghjklyxcvbnm";
        CHECK(expected == stringutil::asLower("qwertzuiopasdfghjklyxcvbnm"sv));
        CHECK(expected == stringutil::asLower("qWeRtZuIoPaSdFgHjKlYxCvBnM"sv));
        CHECK(expected == stringutil::asLower("QWERTZUIOPASDFGHJKLYXCVBNM"sv));
        CHECK(std::string("1234567890+*ç%&/()=") == stringutil::asLower("1234567890+*ç%&/()="sv));
    }

    TEST_CASE("stringutil::toLowerInPlace") {
        std::string expected = "qwertzuiopasdfghjklyxcvbnm";
        std::string actual = "qwertzuiopasdfghjklyxcvbnm";
        stringutil::toLowerInPlace(actual.data());
        CHECK(expected == actual);
        actual = "qWeRtZuIoPaSdFgHjKlYxCvBnM";
        stringutil::toLowerInPlace(actual.data());
        CHECK(expected == actual);
        actual = "QWERTZUIOPASDFGHJKLYXCVBNM";
        stringutil::toLowerInPlace(actual.data());
        CHECK(expected == actual);
    }

    TEST_CASE("stringutil::asUpper") {
        std::string expected = "QWERTZUIOPASDFGHJKLYXCVBNM";
        CHECK(expected == stringutil::asUpper("qwertzuiopasdfghjklyxcvbnm"sv));
        CHECK(expected == stringutil::asUpper("qWeRtZuIoPaSdFgHjKlYxCvBnM"sv));
        CHECK(expected == stringutil::asUpper("QWERTZUIOPASDFGHJKLYXCVBNM"sv));
        CHECK(std::string("1234567890+*ç%&/()=") == stringutil::asUpper("1234567890+*ç%&/()="sv));
    }

    TEST_CASE("stringutil::toUpperInPlace") {
        std::string expected = "QWERTZUIOPASDFGHJKLYXCVBNM";
        std::string actual = "QWERTZUIOPASDFGHJKLYXCVBNM";
        stringutil::toUpperInPlace(actual.data());
        CHECK(expected == actual);
        actual = "qWeRtZuIoPaSdFgHjKlYxCvBnM";
        stringutil::toUpperInPlace(actual.data());
        CHECK(expected == actual);
        actual = "qwertzuiopasdfghjklyxcvbnm";
        stringutil::toUpperInPlace(actual.data());
        CHECK(expected == actual);
    }

    TEST_CASE("stringutil::trim(std::string)") {
        using namespace std::string_literals;
        CHECK(stringutil::trim("abc"s) == "abc");
        CHECK(stringutil::trim(" abc"s) == "abc");
        CHECK(stringutil::trim("\tabc"s) == "abc");
        CHECK(stringutil::trim("\nabc"s) == "abc");
        CHECK(stringutil::trim("\n abc"s) == "abc");
        CHECK(stringutil::trim("\n \tabc"s) == "abc");
        CHECK(stringutil::trim("a bc"s) == "a bc");
        CHECK(stringutil::trim(" a bc"s) == "a bc");
        CHECK(stringutil::trim(" a bc "s) == "a bc");
        CHECK(stringutil::trim(" a b c "s) == "a b c");
        CHECK(stringutil::trim(" a\tbc "s) == "a\tbc");
    }

    TEST_CASE("stringutil::trim(std::string_view)") {
        using namespace std::string_view_literals;
        CHECK(stringutil::trim(""sv).empty());
        CHECK(stringutil::trim("a"sv) == "a");
        CHECK(stringutil::trim("abc"sv) == "abc");
        CHECK(stringutil::trim(" abc"sv) == "abc");
        CHECK(stringutil::trim("\tabc"sv) == "abc");
        CHECK(stringutil::trim("\nabc"sv) == "abc");
        CHECK(stringutil::trim("\n abc"sv) == "abc");
        CHECK(stringutil::trim("\n \tabc"sv) == "abc");
        CHECK(stringutil::trim("a bc"sv) == "a bc");
        CHECK(stringutil::trim(" a bc"sv) == "a bc");
        CHECK(stringutil::trim(" a bc "sv) == "a bc");
        CHECK(stringutil::trim(" a b c "sv) == "a b c");
        CHECK(stringutil::trim(" a\tbc "sv) == "a\tbc");
    }

    TEST_CASE("stringutil::replaceAll") {
        std::string str = "aaXXcc";
        SECTION("normal usage") {
            stringutil::replaceAll(str, "XX", "bb");
            CHECK(str == "aabbcc");
        };
        SECTION("from not found") {
            stringutil::replaceAll(str, "YY", "bb");
            CHECK(str == "aaXXcc");
        };
        SECTION("from at the beginning") {
            stringutil::replaceAll(str, "aa", "zz");
            CHECK(str == "zzXXcc");
        };
        SECTION("from at the end") {
            stringutil::replaceAll(str, "cc", "ZZ");
            CHECK(str == "aaXXZZ");
        };
        SECTION("from larger than to") {
            stringutil::replaceAll(str, "XX", "Z");
            CHECK(str == "aaZcc");
        };
        SECTION("from smaller than to") {
            stringutil::replaceAll(str, "XX", "ZZZ");
            CHECK(str == "aaZZZcc");
        };
        SECTION("multiple replacements") {
            stringutil::replaceAll(str, "a", "Y");
            CHECK(str == "YYXXcc");
        };
        SECTION("multiple replacements, from smaller than to") {
            stringutil::replaceAll(str, "a", "YZ");
            CHECK(str == "YZYZXXcc");
        };
        SECTION("str empty") {
            str = "";
            stringutil::replaceAll(str, "a", "b");
            CHECK(str == "");
        };
        SECTION("from empty") {
            stringutil::replaceAll(str, "", "a");
            CHECK(str == "aaXXcc");
        };
        SECTION("to empty") {
            stringutil::replaceAll(str, "XX", "");
            CHECK(str == "aacc");
        }
    }

    TEST_CASE("stringutil::replaceChar") {
        CHECK(stringutil::replaceChar("aBc", 'B', 'b') == "abc");
        CHECK(stringutil::replaceChar("abc", 'X', 'x') == "abc");
        CHECK(stringutil::replaceChar("abc", 'a', 'A') == "Abc");
        CHECK(stringutil::replaceChar("abcabc", 'a', 'A') == "AbcAbc");
        CHECK(stringutil::replaceChar("", 'a', 'A').empty());
    }

    TEST_CASE("stringutil::formatBytesValue") {
        CHECK(stringutil::formatBytesValue(1) == "1B");
        CHECK(stringutil::formatBytesValue(10) == "10B");
        CHECK(stringutil::formatBytesValue(100) == "100B");
        CHECK(stringutil::formatBytesValue(1023) == "1023B");
        CHECK(stringutil::formatBytesValue(1024) == "1.00KB");
        CHECK(stringutil::formatBytesValue(1025) == "1.00KB");
        CHECK(stringutil::formatBytesValue(10456) == "10.2KB");
        CHECK(stringutil::formatBytesValue(104567) == "102KB");
        CHECK(stringutil::formatBytesValue(1045678) == "1021KB");
        CHECK(stringutil::formatBytesValue(10456789) == "9.97MB");

        CHECK(stringutil::formatBytesValue(1ull << 10) == "1.00KB");
        CHECK(stringutil::formatBytesValue(1ull << 20) == "1.00MB");
        CHECK(stringutil::formatBytesValue(1ull << 30) == "1.00GB");
        CHECK(stringutil::formatBytesValue(1ull << 40) == "1.00TB");
        CHECK(stringutil::formatBytesValue(1ull << 50) == "1024TB");
        CHECK(stringutil::formatBytesValue(1ull << 51) == "2048TB");
    }

    //these are macros because when using lambdas the catch error output is less readable
    #define CHECK_EQUALS_ALPHANUM(a, b)          \
    CHECK(stringutil::equalsAlphanum(a, b)); \
    CHECK(stringutil::equalsAlphanum(b, a));
    #define CHECK_FALSE_EQUALS_ALPHANUM(a, b)          \
    CHECK_FALSE(stringutil::equalsAlphanum(a, b)); \
    CHECK_FALSE(stringutil::equalsAlphanum(b, a));

    TEST_CASE("stringutil::equalsAlphanum") {
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

    TEST_CASE("stringutil::containsIgnoreCase") {
        CHECK(stringutil::containsIgnoreCase("abcdefg", "cde"));
        CHECK(stringutil::containsIgnoreCase("abcdefg", "CdE"));
        CHECK(stringutil::containsIgnoreCase("aBcDeFg", "CdE"));
        CHECK(stringutil::containsIgnoreCase("aBcDeFg", "C"));
        CHECK(stringutil::containsIgnoreCase("aBcDeFg", "CD"));
        CHECK(stringutil::containsIgnoreCase("aBcDeFg", "ab"));
        CHECK(stringutil::containsIgnoreCase("aBcDeFg", "fg"));
        CHECK(stringutil::containsIgnoreCase("aBcDeFg", "aBcDeFg"));
        CHECK_FALSE(stringutil::containsIgnoreCase("aBcDeFg", "df"));
        CHECK_FALSE(stringutil::containsIgnoreCase("aBcDeFg", "aBcDeFgHiJ"));
    }

    TEST_CASE("stringutil::splitByChar") {
        CHECK(stringutil::splitByChar("hello", ' ') == std::vector<std::string_view>({"hello"}));
        CHECK(stringutil::splitByChar("hello world", ' ') == std::vector<std::string_view>({"hello", "world"}));
        CHECK(stringutil::splitByChar("hello world xyz", ' ') == std::vector<std::string_view>({"hello", "world", "xyz"}));
        CHECK(stringutil::splitByChar("hello    world", ' ') == std::vector<std::string_view>({"hello", "world"}));
    }
}
