#include "../../helpers/fraction.h"
#include "catch2/catch.hpp"

namespace bricksim {
    TEST_CASE("fractionConstructor") {
        const Fraction half(1, 2);
        CHECK(half.getA() == 1);
        CHECK(half.getB() == 2);

        const Fraction third(2, 6);
        CHECK(third.getA() == 1);
        CHECK(third.getB() == 3);

        const Fraction thirdCopy(third);// NOLINT(performance-unnecessary-copy-initialization)
        CHECK(thirdCopy.getA() == 1);
        CHECK(thirdCopy.getB() == 3);
    }

    TEST_CASE("fractionSimplify") {
        CHECK(Fraction(1, 2) == Fraction(2, 4));
        CHECK(Fraction(1, 2) == Fraction(123, 246));
        CHECK(Fraction(2, 2) == Fraction(1, 1));
        CHECK(Fraction(2, 1) == Fraction(2, 1));
        CHECK(Fraction(4, 2) == Fraction(2, 1));
    }

    TEST_CASE("fractionCompareToFraction") {
        CHECK(Fraction(1, 2) == Fraction(1, 2));
        CHECK(Fraction(1, 1) == Fraction(1, 1));
        CHECK_FALSE(Fraction(1, 2) == Fraction(1, 1));
        CHECK_FALSE(Fraction(7, 2) == Fraction(5, 3));

        CHECK(Fraction(1, 2) != Fraction(1, 1));
        CHECK(Fraction(2, 1) != Fraction(1, 1));
        CHECK(Fraction(2, 1) != Fraction(3, 1));
        CHECK(Fraction(2, 4) != Fraction(3, 4));
        CHECK_FALSE(Fraction(2, 4) != Fraction(1, 2));

        CHECK(Fraction(1, 2) < Fraction(1, 1));
        CHECK(Fraction(3, 2) < Fraction(5, 3));
        CHECK_FALSE(Fraction(3, 7) < Fraction(1, 3));
        CHECK_FALSE(Fraction(1, 7) < Fraction(2, 33));

        CHECK(Fraction(1, 1) <= Fraction(1, 1));
        CHECK(Fraction(1, 2) <= Fraction(1, 1));
        CHECK(Fraction(1, 2) <= Fraction(3, 4));
        CHECK_FALSE(Fraction(5, 2) <= Fraction(3, 4));

        CHECK(Fraction(5, 2) >= Fraction(3, 4));
        CHECK(Fraction(1, 2) >= Fraction(2, 4));
        CHECK_FALSE(Fraction(1, 8) >= Fraction(2, 4));

        CHECK(Fraction(1, 2) > Fraction(2, 5));
        CHECK(Fraction(6, 7) > Fraction(5, 6));
        CHECK_FALSE(Fraction(6, 45) > Fraction(5, 6));
    }

    TEST_CASE("fractionCompareToLong") {
        CHECK(Fraction(4, 2) == 2);
        CHECK(Fraction(1, 1) == 1);
        CHECK_FALSE(Fraction(1, 2) == 1);
        CHECK_FALSE(Fraction(7, 2) == 2);

        CHECK(Fraction(1, 2) != 1);
        CHECK(Fraction(2, 1) != 1);
        CHECK(Fraction(2, 4) != 7);
        CHECK_FALSE(Fraction(2, 1) != 2);

        CHECK(Fraction(1, 2) < 1);
        CHECK(Fraction(3, 2) < 2);
        CHECK_FALSE(Fraction(9, 7) < 1);
        CHECK_FALSE(Fraction(1, 7) < 0);

        CHECK(Fraction(1, 1) <= 1);
        CHECK(Fraction(1, 2) <= 1);
        CHECK(Fraction(1, 2) <= 2);
        CHECK_FALSE(Fraction(5, 2) <= 1);

        CHECK(Fraction(5, 2) >= 2);
        CHECK(Fraction(4, 2) >= 2);
        CHECK_FALSE(Fraction(1, 8) >= 3);

        CHECK(Fraction(9, 2) > 3);
        CHECK(Fraction(15, 7) > 2);
        CHECK_FALSE(Fraction(6, 45) > 2);
    }

    TEST_CASE("fractionAddition") {
        CHECK(Fraction(1, 3) + Fraction(3, 4) == Fraction(13, 12));
        CHECK(Fraction(1, 3) + Fraction(3, 3) == Fraction(4, 3));
        Fraction fr(3, 5);
        fr += Fraction(3, 7);
        CHECK(fr == Fraction(36, 35));

        CHECK(Fraction(1, 2) + 5 == Fraction(11, 2));
        Fraction fr2(5, 6);
        fr2 += 2;
        CHECK(fr2 == Fraction(17, 6));
    }

    TEST_CASE("fractionSubtraction") {
        CHECK(Fraction(4, 3) - Fraction(3, 4) == Fraction(7, 12));
        CHECK(Fraction(7, 3) - Fraction(3, 3) == Fraction(4, 3));
        Fraction fr(3, 5);
        fr -= Fraction(3, 7);
        CHECK(fr == Fraction(6, 35));

        CHECK(Fraction(13, 2) - 5 == Fraction(3, 2));
        Fraction fr2(20, 6);
        fr2 -= 2;
        CHECK(fr2 == Fraction(4, 3));
    }

    TEST_CASE("fractionMultiplication") {
        CHECK(Fraction(1, 2) * Fraction(1, 3) == Fraction(1, 6));
        CHECK(Fraction(7, 2) * Fraction(1, 3) == Fraction(7, 6));
        Fraction fr(4, 3);
        fr *= Fraction(5, 2);
        CHECK(fr == Fraction(10, 3));

        CHECK(Fraction(1, 3) * 2 == Fraction(2, 3));
        CHECK(Fraction(1, 3) * 3 == Fraction(1, 1));
    }

    TEST_CASE("fractionDivision") {
        CHECK(Fraction(4, 3) / Fraction(1, 2) == Fraction(8, 3));
        CHECK(Fraction(4, 3) / Fraction(1, 1) == Fraction(4, 3));
        CHECK(Fraction(1, 3) / Fraction(1, 3) == 1);
        Fraction fr(3, 5);
        fr /= Fraction(8, 54);
        CHECK(fr == Fraction(81, 20));

        CHECK(Fraction(1, 4) / 2 == Fraction(1, 8));
    }
}