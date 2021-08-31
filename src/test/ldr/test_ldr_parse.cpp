#include "../../ldr/file_reader.h"
#include "../../ldr/files.h"
#include "../testing_tools.h"
#include "catch2/catch.hpp"
#include <glm/gtx/normal.hpp>

using namespace bricksim::ldr;

TEST_CASE("parse ldr::SubfileReference") {
    const std::string line = GENERATE("123 1.5 2 3.3 4 5 6 7 8 9 1 2 3 file with spaces.ldr", "123\t 1.5  2\t\t3.3     4 5\t\t\t\t6 7 8 9 1 2 3\t file with spaces.ldr\t");
    const auto sf = SubfileReference(line, false);
    CHECK(sf.color == 123);
    CHECK(sf.x == Approx(1.5f));
    CHECK(sf.y == Approx(2.f));
    CHECK(sf.z == Approx(3.3f));
    CHECK(sf.a == Approx(4.f));
    CHECK(sf.b == Approx(5.f));
    CHECK(sf.c == Approx(6.f));
    CHECK(sf.d == Approx(7.f));
    CHECK(sf.e == Approx(8.f));
    CHECK(sf.f == Approx(9.f));
    CHECK(sf.g == Approx(1.f));
    CHECK(sf.h == Approx(2.f));
    CHECK(sf.i == Approx(3.f));
    CHECK(sf.bfcInverted == false);
    CHECK(sf.filename == "file with spaces.ldr");
}

TEST_CASE("parse ldr::Line") {
    const std::string line = GENERATE("123 1.2 3.4 5.6 -9.8 -7.6 -5.4", "123\t \t1.20          3.4\t5.6 -9.8  -7.6 -5.4    ");
    const auto li = Line(line);
    CHECK(li.color == 123);
    CHECK(li.x1 == Approx(1.2f));
    CHECK(li.y1 == Approx(3.4f));
    CHECK(li.z1 == Approx(5.6f));
    CHECK(li.x2 == Approx(-9.8f));
    CHECK(li.y2 == Approx(-7.6f));
    CHECK(li.z2 == Approx(-5.4f));
}

TEST_CASE("parse ldr::Triangle") {
    const std::string line = GENERATE("123 1 2 3 4 5 6 9 8 7");
    const WindingOrder wo = GENERATE(CCW, CW);
    const auto tri = Triangle(line, wo);
    auto actualNormal = glm::triangleNormal(
            glm::vec3(tri.x1, tri.y1, tri.z1),
            glm::vec3(tri.x2, tri.y2, tri.z2),
            glm::vec3(tri.x3, tri.y3, tri.z3));
    const auto expectedNormal = (wo == CW ? -1.f : 1.f) * glm::vec3(-0.4082482905f, 0.8164965809f, -0.4082482905f);
    //we can only check the normal because p1;p2;p3 is the same as p3;p1;p2 for example
    CHECK(actualNormal == ApproxVec(expectedNormal));

    CHECK(tri.color == 123);
}

TEST_CASE("ldr::readSimpleFile 1") {
    const std::string filename = GENERATE("filename.ldr", "filename.dat", "filename.mpd");
    const auto type = GENERATE(MODEL, MPD_SUBFILE, PART, SUBPART, PRIMITIVE);
    const auto file = readSimpleFile(filename, "0 Title123\n0 Comment 1\n0 Comment 2", type);
    REQUIRE(file->elements.size() == 2);
    CHECK(file->elements[0]->getLdrLine() == "0 Comment 1");
    CHECK(file->elements[1]->getLdrLine() == "0 Comment 2");
    CHECK(file->metaInfo.type == type);
    CHECK(file->metaInfo.name == filename);
    CHECK(file->metaInfo.title == "Title123");
}

TEST_CASE("ldr::readSimpleFile with MetaInfo") {
    const auto file = readSimpleFile("filename.ldr",
                                     "0 TitleXYZ\n"
                                     "0 Name: NameXYZ\n"
                                     "0 Author: AuthorXYZ\n"
                                     "0 !LDRAW_ORG Part UPDATE 2021-123\n"
                                     "0 !LICENSE LicenseXYZ\n"
                                     "0 !CATEGORY CategoryXYZ\n"
                                     "0 !KEYWORDS KeywordX, KeywordY\n"
                                     "0 !KEYWORDS KeywordZ\n"
                                     "0 !HISTORY HistoryX\n"
                                     "0 !HISTORY HistoryY\n"
                                     "0 Content",
                                     PART);
    CHECK(file->metaInfo.title == "TitleXYZ");
    CHECK(file->metaInfo.name == "NameXYZ");
    CHECK(file->metaInfo.author == "AuthorXYZ");
    CHECK(file->metaInfo.fileTypeLine == "Part UPDATE 2021-123");
    CHECK(file->metaInfo.license == "LicenseXYZ");
    CHECK(file->metaInfo.headerCategory.value_or("") == "CategoryXYZ");
    CHECK(file->metaInfo.keywords == bricksim::oset_t<std::string>({"KeywordX", "KeywordY", "KeywordZ"}));
    CHECK(file->metaInfo.history == std::vector<std::string>({"HistoryX", "HistoryY"}));
    CHECK(file->elements.size() == 1);
    CHECK(file->elements[0]->getLdrLine()=="0 Content");
}