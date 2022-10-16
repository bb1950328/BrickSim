#include "../../connection/ldcad_snap_metas.h"
#include "../testing_tools.h"

using namespace bricksim::connection::ldcad_snap_meta;

/*
SNAP_FGR [genderOfs=M] [seq=4 14 4 16 4 16 4 14 4] [radius=4] [pos=0 -4 6] [ori=0 -1 0 1 0 0 0 0 1]
SNAP_INCL [ref=wpin.dat]
SNAP_CYL [gender=F] [caps=one] [secs=R 6 20] [slide=true] [pos=0 24 0]
SNAP_CYL [gender=M] [caps=one] [secs=R 4 6] [pos=0 -22.72 -5.04] [ori=-1 0 0 0 0.6 -0.8 0 -0.8 -0.6]
*/

TEST_CASE("Reader::readLine 0") {
    const auto metaLine = Reader::readLine("something else");
    CHECK(std::holds_alternative<std::monostate>(metaLine.data));
}

TEST_CASE("Reader::readLine 1") {
    const auto metaLine = Reader::readLine("SNAP_CLEAR");
    CHECK(std::holds_alternative<ClearCommand>(metaLine.data));
    CHECK_FALSE(std::get<ClearCommand>(metaLine.data).id.has_value());
}

TEST_CASE("Reader::readLine 2") {
    const auto metaLine = Reader::readLine("SNAP_CLEAR [id=blabla]");
    CHECK(std::holds_alternative<ClearCommand>(metaLine.data));
    const auto& command = std::get<ClearCommand>(metaLine.data);
    REQUIRE(command.id.has_value());
    CHECK(command.id.value() == "blabla");
}
