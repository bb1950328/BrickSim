#include "../../helpers/almost_comparations.h"
#include "catch2/catch_test_macros.hpp"

namespace bricksim {
    TEST_CASE("almostGreater") {
        CHECK(almostGreater(8, 7));
        CHECK_FALSE(almostGreater(6, 7));
        CHECK(almostGreater(8.0, 8.01, 0.1));
        CHECK(almostGreater(8.01, 8.0, 0.1));
        CHECK_FALSE(almostGreater(8.01, 9.0, 0.1));
    }

    TEST_CASE("almostLess") {
        CHECK(almostLess(7, 8));
        CHECK_FALSE(almostLess(7, 6));
        CHECK(almostLess(8.01, 8.0, 0.1));
        CHECK(almostLess(8.0, 8.01, 0.1));
        CHECK_FALSE(almostLess(9.0, 8.01, 0.1));
    }

    TEST_CASE("almostEqual") {
        CHECK_FALSE(almostEqual(7, 8));
        CHECK_FALSE(almostEqual(7, 6));
        CHECK_FALSE(almostEqual(7.f, 8.f, .1f));
        CHECK_FALSE(almostEqual(7.f, 6.f, .1f));
        CHECK(almostEqual(8.01, 8.0, 0.1));
        CHECK(almostEqual(8.0, 8.01, 0.1));
        CHECK(almostEqual(80, 81, 2));
    }
}
