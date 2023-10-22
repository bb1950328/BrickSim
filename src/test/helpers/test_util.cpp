#include "../../helpers/platform_detection.h"
#include "../../helpers/util.h"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include <fstream>
#include <glm/gtc/epsilon.hpp>
#include <iostream>

namespace bricksim {

    TEST_CASE("util::extendHomeDir and util::replaceSpecialPaths") {
        CHECK(util::extendHomeDir("~/abc") != "~/abc");
        CHECK(util::extendHomeDir("/abc/def") == "/abc/def");
        CHECK(util::extendHomeDir("/abc~def") == "/abc~def");
        CHECK(util::extendHomeDir("/abc~") == "/abc~");
    }

    TEST_CASE("util::biggestValue") {
        CHECK(util::biggestValue(glm::vec2{1000, 1}) == 1000);
        CHECK(util::biggestValue(glm::vec2{1, 1000}) == 1000);

        CHECK(util::biggestValue(glm::vec3{1000, 1, 2}) == 1000);
        CHECK(util::biggestValue(glm::vec3{1, 1000, 2}) == 1000);
        CHECK(util::biggestValue(glm::vec3{1, 2, 1000}) == 1000);

        CHECK(util::biggestValue(glm::vec4{1000, 1, 2, 3}) == 1000);
        CHECK(util::biggestValue(glm::vec4{1, 1000, 2, 3}) == 1000);
        CHECK(util::biggestValue(glm::vec4{1, 2, 1000, 3}) == 1000);
        CHECK(util::biggestValue(glm::vec4{1, 2, 3, 1000}) == 1000);

        CHECK(util::biggestValue(glm::vec<2, short, glm::defaultp>(1, 2)) == 2);
        CHECK(util::biggestValue(glm::vec<2, unsigned long, glm::defaultp>(static_cast<unsigned long>(-1), 1)) == static_cast<unsigned long>(-1));
    }

    TEST_CASE("util::vectorSum") {
        CHECK(util::vectorSum(glm::vec2{1.0f, 2.0f}) == Catch::Approx(3.0f));
        CHECK(util::vectorSum(glm::vec3{1.0f, 2.0f, 3.0f}) == Catch::Approx(6.0f));
        CHECK(util::vectorSum(glm::vec4{1.0f, 2.0f, 3.0f, 4.0f}) == Catch::Approx(10.0f));
    }

    template<auto val>
    using value_wrapper = std::integral_constant<decltype(val), val>;

    TEMPLATE_TEST_CASE("util::memeqzero", "", value_wrapper<5>, value_wrapper<50>, value_wrapper<100>) {
        uint8_t buf1[TestType::value] = {0};
        REQUIRE(util::memeqzero(buf1, TestType::value));

        SECTION("modify first byte") {
            buf1[0] = 1;
            CHECK_FALSE(util::memeqzero(buf1, TestType::value));
        };
        SECTION("modify middle byte") {
            buf1[2] = 1;
            CHECK_FALSE(util::memeqzero(buf1, TestType::value));
        };
        SECTION("modify last byte") {
            buf1[TestType::value - 1] = 1;
            CHECK_FALSE(util::memeqzero(buf1, TestType::value));
        }
    }

    TEST_CASE("util::readFileToString") {
        auto filename = std::filesystem::temp_directory_path() / "BrickSimUnitTest.txt";

        std::ofstream out(filename);
        out << "Hello World!";
        out.close();

        REQUIRE(util::readFileToString(filename) == "Hello World!");

        std::filesystem::remove(filename);
    }

    TEST_CASE("util::minForEachComponent") {
        CHECK(util::cwiseMin(glm::vec2{1, 1}, glm::vec2{2, 2}) == glm::vec2(1, 1));
        CHECK(util::cwiseMin(glm::vec2{1, 2}, glm::vec2{2, 1}) == glm::vec2(1, 1));

        CHECK(util::cwiseMin(glm::vec3{1, 2, 3}, glm::vec3{3, 2, 1}) == glm::vec3(1, 2, 1));

        CHECK(util::cwiseMin(glm::vec4{1, 2, 3, 4}, glm::vec4{5, 4, 3, 2}) == glm::vec4(1, 2, 3, 2));
    }

    TEST_CASE("util::withoutBasePath") {
        std::filesystem::path abc("abc");
        std::filesystem::path def("def");
        std::filesystem::path ghi("ghi");
        CHECK(util::withoutBasePath(abc / def / ghi, abc / def) == ghi);
        CHECK(util::withoutBasePath(abc / def / ghi, abc) == def / ghi);
        CHECK(util::withoutBasePath(abc / def / abc, abc) == def / abc);
        CHECK(util::withoutBasePath(abc / abc / def / abc, abc) == abc / def / abc);
        CHECK(util::withoutBasePath(abc, abc) == "");
        CHECK(util::withoutBasePath(abc, def) == abc);
        CHECK(util::withoutBasePath(abc / def, def) == abc / def);
    }

    TEST_CASE("util::combinedHash") {
        CHECK(util::combinedHash(123, 456) != hash<int>()(123));
        CHECK(util::combinedHash(123, 456) != hash<int>()(456));
        CHECK(util::combinedHash(123, 456) != util::combinedHash(456, 123));

        CHECK(util::combinedHash(123, 456, 789) != util::combinedHash(123, 456));
        CHECK(util::combinedHash(123, 456, 789) != util::combinedHash(456, 789));
    }

    TEST_CASE("util::ScopeVarBackup1") {
        const int initialValue = GENERATE(2, 3);
        const int temporaryValue = GENERATE(3, 4);
        int var = initialValue;
        {
            util::ScopeVarBackup backup(var);
            var = temporaryValue;
        }
        CHECK(var == initialValue);
    }

    TEST_CASE("util::ScopeVarBackup2") {
        const int initialValue = GENERATE(2, 3);
        const int temporaryValue = GENERATE(3, 4);
        int var = initialValue;
        {
            util::ScopeVarBackup backup(var, temporaryValue);
            CHECK(var == temporaryValue);
        }
        CHECK(var == initialValue);
    }
}
