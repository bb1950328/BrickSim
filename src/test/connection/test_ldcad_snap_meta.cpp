#include "../../connection/ldcad_snap_meta.h"
#include "../testing_tools.h"

using namespace bricksim::connection::ldcad_snap_meta;

TEST_CASE("Reader::readLine 0") {
    const auto command = Reader::readLine("something else");
    CHECK(command == nullptr);
}

TEST_CASE("Reader::readLine 1") {
    const auto command = std::dynamic_pointer_cast<ClearCommand>(Reader::readLine("SNAP_CLEAR"));
    REQUIRE(command != nullptr);
    CHECK_FALSE(command->id.has_value());
}

TEST_CASE("Reader::readLine 2") {
    const auto command = std::dynamic_pointer_cast<ClearCommand>(Reader::readLine("SNAP_CLEAR [id=blabla]"));
    REQUIRE(command != nullptr);
    REQUIRE(command->id.has_value());
    CHECK(command->id.value() == "blabla");
}

TEST_CASE("Reader::readLine 3") {
    const auto command = std::dynamic_pointer_cast<FgrCommand>(Reader::readLine("SNAP_FGR [genderOfs=M] [seq=4 14 4 16 4 16 4 14 4] [radius=4] [pos=0 -4 6] [ori=0 -1 0 1 0 0 0 0 1]"));
    REQUIRE(command != nullptr);
    CHECK(command->genderOfs == Gender::M);
    CHECK(command->seq == std::vector<float>({4, 14, 4, 16, 4, 16, 4, 14, 4}));
    CHECK(command->radius == Catch::Approx(4));
    REQUIRE(command->pos.has_value());
    CHECK(command->pos.value() == ApproxVec(glm::vec3(0, -4, 6)));
    CHECK(command->ori.value() == ApproxMat(glm::mat3(0, -1, 0, 1, 0, 0, 0, 0, 1)));
}

TEST_CASE("Reader::readLine 4") {
    const auto command = std::dynamic_pointer_cast<InclCommand>(Reader::readLine("SNAP_INCL [ref=wpin.dat]"));
    REQUIRE(command != nullptr);
    CHECK(command->ref == "wpin.dat");
    CHECK_FALSE(command->id.has_value());
    CHECK_FALSE(command->pos.has_value());
    CHECK_FALSE(command->ori.has_value());
    CHECK_FALSE(command->scale.has_value());
    CHECK_FALSE(command->grid.has_value());
}

TEST_CASE("Reader::readLine 5") {
    const auto command = std::dynamic_pointer_cast<CylCommand>(Reader::readLine("SNAP_CYL [gender=F] [caps=one] [secs=R 6 20] [slide=true] [pos=0 24 0]"));
    REQUIRE(command != nullptr);
    CHECK_FALSE(command->id.has_value());
    CHECK_FALSE(command->group.has_value());
    REQUIRE(command->pos.has_value());
    CHECK(command->pos.value() == ApproxVec(glm::vec3(0, 24, 0)));
    CHECK_FALSE(command->ori.has_value());
    CHECK(command->scale == ScaleType::NONE);
    CHECK(command->mirror == MirrorType::NONE);
    CHECK(command->gender == Gender::F);
    CHECK(command->secs == std::vector<CylShapeBlock>{CylShapeBlock{CylShapeVariant::R, 6, 20}});
    CHECK(command->caps == CylCaps::ONE);
    CHECK_FALSE(command->grid.has_value());
    CHECK_FALSE(command->center);
    CHECK(command->slide);
}

TEST_CASE("Reader::readLine 6") {
    const auto command = std::dynamic_pointer_cast<CylCommand>(Reader::readLine("SNAP_CYL [gender=M] [caps=two] [secs=R 4 6] [pos=0 -22.72 -5.04] [ori=-1 0 0 0 0.6 -0.8 0 -0.8 -0.6]"));
    REQUIRE(command != nullptr);
    CHECK_FALSE(command->id.has_value());
    CHECK_FALSE(command->group.has_value());
    REQUIRE(command->pos.has_value());
    CHECK(command->pos.value() == ApproxVec(glm::vec3(0, -22.72, -5.04)));
    REQUIRE(command->ori.has_value());
    CHECK(command->ori.value() == ApproxMat(glm::mat3(-1, 0, 0, 0, 0.6, -0.8, 0, -0.8, -0.6)));
    CHECK(command->scale == ScaleType::NONE);
    CHECK(command->mirror == MirrorType::NONE);
    CHECK(command->gender == Gender::M);
    CHECK(command->secs == std::vector<CylShapeBlock>{CylShapeBlock{CylShapeVariant::R, 4, 6}});
    CHECK(command->caps == CylCaps::TWO);
    CHECK_FALSE(command->grid.has_value());
    CHECK_FALSE(command->center);
    CHECK_FALSE(command->slide);
}