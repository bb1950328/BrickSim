#include "catch2/catch.hpp"
#include "../tools/gears.h"

TEST_CASE("Gear Constants") {
    REQUIRE(gears::WORM_GEAR->numTeeth==1);
}