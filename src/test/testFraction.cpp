#include "catch2/catch.hpp"
#include "../helpers/fraction.h"

TEST_CASE("fractionConstructor") {
    const Fraction half(1, 2);
    REQUIRE(half.getA()==1);
    REQUIRE(half.getB()==2);

    const Fraction third(2, 6);
    REQUIRE(third.getA()==1);
    REQUIRE(third.getB()==3);

    const Fraction thirdCopy(third); // NOLINT(performance-unnecessary-copy-initialization)
    REQUIRE(thirdCopy.getA()==1);
    REQUIRE(thirdCopy.getB()==3);
}

TEST_CASE("fractionSimplify") {
    REQUIRE(Fraction(1, 2)==Fraction(2, 4));
    REQUIRE(Fraction(1, 2)==Fraction(123, 246));
    REQUIRE(Fraction(2, 2)==Fraction(1, 1));
    REQUIRE(Fraction(2, 1)==Fraction(2, 1));
    REQUIRE(Fraction(4, 2)==Fraction(2, 1));
}

TEST_CASE("fractionCompareToFraction") {
    REQUIRE(Fraction(1, 2)==Fraction(1, 2));
    REQUIRE(Fraction(1, 1)==Fraction(1, 1));
    REQUIRE_FALSE(Fraction(1, 2)==Fraction(1, 1));
    REQUIRE_FALSE(Fraction(7, 2)==Fraction(5, 3));

    REQUIRE(Fraction(1, 2)!=Fraction(1, 1));
    REQUIRE(Fraction(2, 1)!=Fraction(1, 1));
    REQUIRE(Fraction(2, 1)!=Fraction(3, 1));
    REQUIRE(Fraction(2, 4)!=Fraction(3, 4));
    REQUIRE_FALSE(Fraction(2, 4)!=Fraction(1, 2));

    REQUIRE(Fraction(1, 2)<Fraction(1, 1));
    REQUIRE(Fraction(3, 2)<Fraction(5, 3));
    REQUIRE_FALSE(Fraction(3, 7)<Fraction(1, 3));
    REQUIRE_FALSE(Fraction(1, 7)<Fraction(2, 33));

    REQUIRE(Fraction(1, 1)<=Fraction(1, 1));
    REQUIRE(Fraction(1, 2)<=Fraction(1, 1));
    REQUIRE(Fraction(1, 2)<=Fraction(3, 4));
    REQUIRE_FALSE(Fraction(5, 2)<=Fraction(3, 4));

    REQUIRE(Fraction(5, 2)>=Fraction(3, 4));
    REQUIRE(Fraction(1, 2)>=Fraction(2, 4));
    REQUIRE_FALSE(Fraction(1, 8)>=Fraction(2, 4));

    REQUIRE(Fraction(1, 2)>Fraction(2, 5));
    REQUIRE(Fraction(6, 7)>Fraction(5, 6));
    REQUIRE_FALSE(Fraction(6, 45)>Fraction(5, 6));
}

TEST_CASE("fractionCompareToLong") {
    REQUIRE(Fraction(4, 2)==2);
    REQUIRE(Fraction(1, 1)==1);
    REQUIRE_FALSE(Fraction(1, 2)==1);
    REQUIRE_FALSE(Fraction(7, 2)==2);

    REQUIRE(Fraction(1, 2)!=1);
    REQUIRE(Fraction(2, 1)!=1);
    REQUIRE(Fraction(2, 4)!=7);
    REQUIRE_FALSE(Fraction(2, 1)!=2);

    REQUIRE(Fraction(1, 2)<1);
    REQUIRE(Fraction(3, 2)<2);
    REQUIRE_FALSE(Fraction(9, 7)<1);
    REQUIRE_FALSE(Fraction(1, 7)<0);

    REQUIRE(Fraction(1, 1)<=1);
    REQUIRE(Fraction(1, 2)<=1);
    REQUIRE(Fraction(1, 2)<=2);
    REQUIRE_FALSE(Fraction(5, 2)<=1);

    REQUIRE(Fraction(5, 2)>=2);
    REQUIRE(Fraction(4, 2)>=2);
    REQUIRE_FALSE(Fraction(1, 8)>=3);

    REQUIRE(Fraction(9, 2)>3);
    REQUIRE(Fraction(15, 7)>2);
    REQUIRE_FALSE(Fraction(6, 45)>2);
}

TEST_CASE("fractionAddition") {
    REQUIRE(Fraction(1, 3)+Fraction(3, 4)==Fraction(13, 12));
    REQUIRE(Fraction(1, 3)+Fraction(3, 3)==Fraction(4, 3));
    Fraction fr(3, 5);
    fr += Fraction(3, 7);
    REQUIRE(fr==Fraction(36, 35));

    REQUIRE(Fraction(1, 2)+5==Fraction(11, 2));
    Fraction fr2(5, 6);
    fr2 += 2;
    REQUIRE(fr2==Fraction(17, 6));
}

TEST_CASE("fractionSubtraction") {
    REQUIRE(Fraction(4, 3)-Fraction(3, 4)==Fraction(7, 12));
    REQUIRE(Fraction(7, 3)-Fraction(3, 3)==Fraction(4, 3));
    Fraction fr(3, 5);
    fr -= Fraction(3, 7);
    REQUIRE(fr==Fraction(6, 35));

    REQUIRE(Fraction(13, 2)-5==Fraction(3, 2));
    Fraction fr2(20, 6);
    fr2 -= 2;
    REQUIRE(fr2==Fraction(4, 3));
}

TEST_CASE("fractionMultiplication") {
    REQUIRE(Fraction(1, 2)*Fraction(1, 3)==Fraction(1, 6));
    REQUIRE(Fraction(7, 2)*Fraction(1, 3)==Fraction(7, 6));
    Fraction fr(4, 3);
    fr *= Fraction(5, 2);
    REQUIRE(fr==Fraction(10, 3));

    REQUIRE(Fraction(1, 3)*2==Fraction(2, 3));
    REQUIRE(Fraction(1, 3)*3==Fraction(1, 1));
}

TEST_CASE("fractionDivision") {
    REQUIRE(Fraction(4, 3) / Fraction(1, 2)==Fraction(8, 3));
    REQUIRE(Fraction(4, 3) / Fraction(1, 1)==Fraction(4, 3));
    REQUIRE(Fraction(1, 3) / Fraction(1, 3)==1);
    Fraction fr(3, 5);
    fr /= Fraction(8, 54);
    REQUIRE(fr==Fraction(81, 20));

    REQUIRE(Fraction(1, 4) / 2 == Fraction(1, 8));
}

