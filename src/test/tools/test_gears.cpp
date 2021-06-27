#include "catch2/catch.hpp"
#include "../../tools/gears.h"
namespace bricksim {
    TEST_CASE("Gear Constants") {
        CHECK(gears::WORM_GEAR->numTeeth == 1);
        CHECK(gears::WORM_GEAR->radiusLDU == 10);
        CHECK(gears::WORM_GEAR->type == gears::GearType::WORM);
    }

    TEST_CASE("GearPair") {
        gears::GearPair pair(gears::WORM_GEAR, gears::GEAR_8T);
        CHECK(pair.getDriver() == gears::WORM_GEAR);
        CHECK(pair.getFollower() == gears::GEAR_8T);
        CHECK(pair.getRatio() == Fraction(1, 8));
    }

}

//todo more tests