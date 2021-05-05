#include "catch2/catch.hpp"
#include "../tools/gears.h"

TEST_CASE("Gear Constants") {
    REQUIRE(gears::WORM_GEAR->numTeeth==1);
    REQUIRE(gears::WORM_GEAR->radiusLDU==10);
    REQUIRE(gears::WORM_GEAR->type==gears::GearType::WORM);
}

TEST_CASE("GearPair") {
    gears::GearPair pair(gears::WORM_GEAR, gears::GEAR_8T);
    REQUIRE(pair.getDriver()==gears::WORM_GEAR);
    REQUIRE(pair.getFollower()==gears::GEAR_8T);
    REQUIRE(pair.getRatio()==Fraction(1, 24));
}