#include "../../ldr/files.h"
#include "catch2/catch_test_macros.hpp"
#include <sstream>

using namespace bricksim::ldr;

TEST_CASE("ldr::CommentOrMetaElement to string") {
    CHECK(CommentOrMetaElement("abc").getLdrLine() == "0 abc");
    CHECK(CommentOrMetaElement("abc def").getLdrLine() == "0 abc def");
}

TEST_CASE("ldr::SubfileReference to string") {
    CHECK(SubfileReference("123  1  2   3 4 5 6 7 8 9 10 11 12   subfile with space.ldr", false).getLdrLine() == "1 123 1 2 3 4 5 6 7 8 9 10 11 12 subfile with space.ldr");
}

TEST_CASE("ldr::Line to string") {
    CHECK(Line("123 1 2 3 4 5 6").getLdrLine() == "2 123 1 2 3 4 5 6");
}

TEST_CASE("ldr::Triangle to string") {
    CHECK(Triangle("123 1 2 3 4 5 6 7 8 9", WindingOrder::CCW).getLdrLine() == "3 123 1 2 3 4 5 6 7 8 9");
}

TEST_CASE("ldr::Quadrilateral to string") {
    CHECK(Quadrilateral("123 1 2 3 4 5 6 7 8 9 10 11 12", WindingOrder::CCW).getLdrLine() == "4 123 1 2 3 4 5 6 7 8 9 10 11 12");
}

TEST_CASE("ldr::OptionalLine to string") {
    CHECK(OptionalLine("123 1 2 3 4 5 6 7 8 9 10 11 12").getLdrLine() == "5 123 1 2 3 4 5 6 7 8 9 10 11 12");
}

TEST_CASE("ldr::FileMetaInfo to string") {
    FileMetaInfo metaInfo;
    metaInfo.title = "ABC";
    metaInfo.name = "DEF";
    metaInfo.author = "GHI";
    metaInfo.keywords.insert({"JKL", "MNO", "PQR"});
    metaInfo.history.assign({"STU", "VWX", "YZ1"});
    metaInfo.license = "234";
    metaInfo.theme = "567";
    metaInfo.fileTypeLine = "890";
    metaInfo.headerCategory = "abc";
    metaInfo.type = FileType::PRIMITIVE;

    std::stringstream ss;
    ss << metaInfo;
    CHECK(ss.str().find("0 ABC") == 0);
    CHECK(ss.str().find("0 Name: DEF") != std::string::npos);
    CHECK(ss.str().find("0 Author: GHI") != std::string::npos);
    CHECK(ss.str().find("0 !KEYWORDS JKL, MNO, PQR") != std::string::npos);
    CHECK(ss.str().find("0 !HISTORY STU") != std::string::npos);
    CHECK(ss.str().find("0 !HISTORY VWX") != std::string::npos);
    CHECK(ss.str().find("0 !HISTORY YZ1") != std::string::npos);
    CHECK(ss.str().find("0 !LICENSE 234") != std::string::npos);
    CHECK(ss.str().find("0 !THEME 567") != std::string::npos);
    CHECK(ss.str().find("0 !LDRAW_ORG 890") != std::string::npos);
    CHECK(ss.str().find("0 !CATEGORY abc") != std::string::npos);
}
