#include "catch2/catch.hpp"
#include "../helpers/util.h"

TEST_CASE("Hello World", "[hello]") {
    REQUIRE(util::endsWith("helloWorld", "World"));
    REQUIRE(util::endsWith("abc", "abc"));
    REQUIRE(util::endsWith("a", "a"));
    REQUIRE_FALSE(util::endsWith("abc", "de"));
}