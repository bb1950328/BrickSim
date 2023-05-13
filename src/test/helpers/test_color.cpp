#include "../../helpers/color.h"
#include "../testing_tools.h"

#include <iostream>

using namespace bricksim;

TEST_CASE("construct color::RGB from (int, int, int)") {
    color::RGB col(1, 2, 3);
    CHECK(col.red == 1);
    CHECK(col.green == 2);
    CHECK(col.blue == 3);
}

TEST_CASE("construct color::RGB from HTML code") {
    color::RGB col("#010203");
    CHECK(col.red == 1);
    CHECK(col.green == 2);
    CHECK(col.blue == 3);
}

TEST_CASE("construct color::RGB from glm::vec3") {
    color::RGB col(glm::vec3(.1f, .2f, .3f));
    CHECK(col.red == 26);
    CHECK(col.green == 51);
    CHECK(col.blue == 77);

    CHECK(color::RGB(glm::vec3(2.f, 0, 0)).red == 255);
    CHECK(color::RGB(glm::vec3(-1.f, 0, 0)).red == 0);
}

TEST_CASE("construct color::RGB from HSV") {
    color::RGB col(color::HSV(11, 22, 33));
    CHECK(col.red == 33);
    CHECK(col.green == 30);
    CHECK(col.blue == 30);
}

TEST_CASE("color::asGlmVector()") {
    const glm::vec3 vec(.2f, .4f, .6f);
    CHECK(color::RGB(vec).asGlmVector() == ApproxVec(vec));
}

TEST_CASE("color::asHtmlCode()") {
    CHECK(color::RGB(11, 22, 33).asHtmlCode() == "#0b1621");
    CHECK(color::RGB(0x02, 0x00, 0x12).asHtmlCode() == "#020012");
    CHECK(color::BLACK.asHtmlCode() == "#000000");
}

TEST_CASE("construct color::HSV from (int, int, int)") {
    color::HSV col(1, 2, 3);
    CHECK(col.hue == 1);
    CHECK(col.saturation == 2);
    CHECK(col.value == 3);
}


TEST_CASE("construct color::HSV from glm::vec3") {
    color::HSV col(glm::vec3(.25f, .5f, .75f));
    CHECK(col.hue==64);
    CHECK(col.saturation == 128);
    CHECK(col.value == 191);
}

