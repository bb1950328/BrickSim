#include "../../ldr/file_reader.h"
#include "../../ldr/files.h"
#include "../testing_tools.h"
#include "catch2/generators/catch_generators_all.hpp"
#include <glm/gtx/normal.hpp>

using namespace bricksim::ldr;

TEST_CASE("parse ldr::SubfileReference") {
    const std::string line = GENERATE("123 1.5 2 3.3 4 5 6 7 8 9 1 2 3 file with spaces.ldr", "123\t 1.5  2\t\t3.3     4 5\t\t\t\t6 7 8 9 1 2 3\t file with spaces.ldr\t");
    const auto sf = SubfileReference(line, false);
    CHECK(sf.color == 123);
    CHECK(sf.x() == Catch::Approx(1.5f));
    CHECK(sf.y() == Catch::Approx(2.f));
    CHECK(sf.z() == Catch::Approx(3.3f));
    CHECK(sf.a() == Catch::Approx(4.f));
    CHECK(sf.b() == Catch::Approx(5.f));
    CHECK(sf.c() == Catch::Approx(6.f));
    CHECK(sf.d() == Catch::Approx(7.f));
    CHECK(sf.e() == Catch::Approx(8.f));
    CHECK(sf.f() == Catch::Approx(9.f));
    CHECK(sf.g() == Catch::Approx(1.f));
    CHECK(sf.h() == Catch::Approx(2.f));
    CHECK(sf.i() == Catch::Approx(3.f));
    CHECK(sf.bfcInverted == false);
    CHECK(sf.filename == "file with spaces.ldr");
}

TEST_CASE("parse ldr::Line") {
    const std::string line = GENERATE("123 1.2 3.4 5.6 -9.8 -7.6 -5.4", "123\t \t1.20          3.4\t5.6 -9.8  -7.6 -5.4    ");
    const auto li = Line(line);
    CHECK(li.color == 123);
    CHECK(li.x1() == Catch::Approx(1.2f));
    CHECK(li.y1() == Catch::Approx(3.4f));
    CHECK(li.z1() == Catch::Approx(5.6f));
    CHECK(li.x2() == Catch::Approx(-9.8f));
    CHECK(li.y2() == Catch::Approx(-7.6f));
    CHECK(li.z2() == Catch::Approx(-5.4f));
}

TEST_CASE("parse ldr::Triangle") {
    const std::string line = GENERATE("123 1 2 3 4 5 6 9 8 7");
    const WindingOrder wo = GENERATE(WindingOrder::CCW, WindingOrder::CW);
    const auto tri = Triangle(line, wo);
    auto actualNormal = glm::triangleNormal(
            glm::vec3(tri.x1(), tri.y1(), tri.z1()),
            glm::vec3(tri.x2(), tri.y2(), tri.z2()),
            glm::vec3(tri.x3(), tri.y3(), tri.z3()));
    const auto expectedNormal = (wo == WindingOrder::CW ? -1.f : 1.f) * glm::vec3(-0.4082482905f, 0.8164965809f, -0.4082482905f);
    //we can only check the normal because p1;p2;p3 is the same as p3;p1;p2 for example
    CHECK(actualNormal == ApproxVec(expectedNormal));

    CHECK(tri.color == 123);
}

TEST_CASE("parse ldr::TexmapStartCommand 1") {
    const std::string line = GENERATE(
            "!TEXMAP START PLANAR 1 2 3 4 5 6 7 8 9 texture.png GLOSSMAP glossmap.png",
            "!TEXMAP NEXT PLANAR 1.0 2.0 3.0 4 5 6 7 8 9 texture.png GLOSSMAP glossmap.png",
            "!TEXMAP START PLANAR 1 2 3 4 5 6 7 8 9 texture.png GLOSSMAP glossmap.png",
            "!TEXMAP\tSTART\tPLANAR\t1\t2\t3\t4\t5\t6\t7\t8\t9\ttexture.png\tGLOSSMAP\tglossmap.png",
            "!TEXMAP\t START  \t\tPLANAR\t \t1  \t 2 \t3 \t   4\t   5\t  \t6\t 7\t 8\t 9\t  texture.png\t  \tGLOSSMAP  \t  glossmap.png\t ");
    const auto command = TexmapStartCommand(line);
    CHECK(command.projectionMethod == bricksim::ldr::TexmapStartCommand::ProjectionMethod::PLANAR);
    CHECK(command.x1() == Catch::Approx(1.f));
    CHECK(command.y1() == Catch::Approx(2.f));
    CHECK(command.z1() == Catch::Approx(3.f));
    CHECK(command.x2() == Catch::Approx(4.f));
    CHECK(command.y2() == Catch::Approx(5.f));
    CHECK(command.z2() == Catch::Approx(6.f));
    CHECK(command.x3() == Catch::Approx(7.f));
    CHECK(command.y3() == Catch::Approx(8.f));
    CHECK(command.z3() == Catch::Approx(9.f));
    CHECK(command.a() == Catch::Approx(0.f));
    CHECK(command.b() == Catch::Approx(0.f));
    CHECK(command.textureFilename == "texture.png");
    CHECK(command.glossmapFileName.value() == "glossmap.png");
}

TEST_CASE("parse ldr::TexmapStartCommand 2") {
    auto cylindricalCommand = TexmapStartCommand("!TEXMAP START CYLINDRICAL 1 2 3 4 5 6 7 8 9 11 texture.png");
    CHECK(cylindricalCommand.projectionMethod == TexmapStartCommand::ProjectionMethod::CYLINDRICAL);
    CHECK(cylindricalCommand.a() == Catch::Approx(11.f));
    CHECK(cylindricalCommand.b() == Catch::Approx(0.f));

    auto sphericalCommand = TexmapStartCommand("!TEXMAP START SPHERICAL 1 2 3 4 5 6 7 8 9 11 22 texture.png");
    CHECK(sphericalCommand.projectionMethod == TexmapStartCommand::ProjectionMethod::SPHERICAL);
    CHECK(sphericalCommand.a() == Catch::Approx(11.f));
    CHECK(sphericalCommand.b() == Catch::Approx(22.f));

    auto textureWithSpacesCommand = TexmapStartCommand("!TEXMAP START PLANAR 1 2 3 4 5 6 7 8 9 tex ture.png");
    CHECK(textureWithSpacesCommand.textureFilename == "tex ture.png");
    CHECK(textureWithSpacesCommand.glossmapFileName.has_value() == false);

    auto textureAndGlossmapWithSpacesCommand = TexmapStartCommand("!TEXMAP START PLANAR 1 2 3 4 5 6 7 8 9 tex ture.png GLOSSMAP gloss map.png");
    CHECK(textureAndGlossmapWithSpacesCommand.textureFilename == "tex ture.png");
    CHECK(textureAndGlossmapWithSpacesCommand.glossmapFileName.value() == "gloss map.png");

    auto textureWithGlossmapInPathCommand = TexmapStartCommand("!TEXMAP START PLANAR 1 2 3 4 5 6 7 8 9 /home/user/GLOSSMAPS/texture.png");
    CHECK(textureWithGlossmapInPathCommand.textureFilename == "/home/user/GLOSSMAPS/texture.png");
    CHECK(textureWithGlossmapInPathCommand.glossmapFileName.has_value() == false);
}

TEST_CASE("ldr::readSimpleFile 1") {
    const std::string filename = GENERATE("filename.ldr", "filename.dat", "filename.mpd");
    const auto type = GENERATE(FileType::MODEL, FileType::MPD_SUBFILE, FileType::PART, FileType::SUBPART, FileType::PRIMITIVE);
    const auto file = readSimpleFile(nullptr, filename, "", type, "0 Title123\n0 Comment 1\n0 Comment 2", {});
    REQUIRE(file->elements.size() == 2);
    CHECK(file->elements[0]->getLdrLine() == "0 Comment 1");
    CHECK(file->elements[1]->getLdrLine() == "0 Comment 2");
    CHECK(file->metaInfo.type == type);
    CHECK(file->metaInfo.name == filename);
    CHECK(file->metaInfo.title == "Title123");
}

TEST_CASE("ldr::readSimpleFile with MetaInfo") {
    const auto file = readSimpleFile(nullptr,
                                     "filename.ldr",
                                     "",
                                     FileType::PART,
                                     "0 TitleXYZ\n"
                                     "0 Name: filename.ldr\n"
                                     "0 Author: AuthorXYZ\n"
                                     "0 !LDRAW_ORG Part UPDATE 2021-123\n"
                                     "0 !LICENSE LicenseXYZ\n"
                                     "0 !CATEGORY CategoryXYZ\n"
                                     "0 !KEYWORDS KeywordX, KeywordY\n"
                                     "0 !KEYWORDS KeywordZ\n"
                                     "0 !HISTORY HistoryX\n"
                                     "0 !HISTORY HistoryY\n"
                                     "0 Content",
                                     {});
    CHECK(file->metaInfo.title == "TitleXYZ");
    CHECK(file->metaInfo.name == "filename.ldr");
    CHECK(file->metaInfo.author == "AuthorXYZ");
    CHECK(file->metaInfo.fileTypeLine == "Part UPDATE 2021-123");
    CHECK(file->metaInfo.license == "LicenseXYZ");
    CHECK(file->metaInfo.headerCategory.value_or("") == "CategoryXYZ");
    CHECK(file->metaInfo.keywords == bricksim::oset_t<std::string>({"KeywordX", "KeywordY", "KeywordZ"}));
    CHECK(file->metaInfo.history == std::vector<std::string>({"HistoryX", "HistoryY"}));
    CHECK(file->elements.size() == 1);
    CHECK(file->elements[0]->getLdrLine()=="0 Content");
}
