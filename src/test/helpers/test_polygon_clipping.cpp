#include "../../helpers/earcut_hpp_with_glm.h"
#include "../../helpers/geometry.h"
#include "../../helpers/polygon_clipping.h"
#include "../testing_tools.h"
#include "catch2/catch.hpp"
#include <array>

namespace bricksim {

    //https://www.geogebra.org/calculator/eyrnn38k
    const std::vector<glm::vec2> POLYGON1 = consistentStartOfCircularList<2>({{0, -1}, {6, 2}, {-2, 5}});
    const std::vector<glm::vec2> POLYGON2 = consistentStartOfCircularList<2>({{3, 6}, {2, -3}, {10, -2}, {10, 5}});
    const std::vector<glm::vec2> INTERSECTION_1_2_CCW = consistentStartOfCircularList<2>({{6.f, 2.f}, {2.6933333333333f, 3.24f}, {2.3529411764706f, 0.1764705882353f}});
    const std::vector<glm::vec2> INTERSECTION_1_2_CW = consistentStartOfCircularList<2>({INTERSECTION_1_2_CCW.rbegin(), INTERSECTION_1_2_CCW.rend()});
    const float POLYGON1_AREA = 21;
    const float POLYGON2_AREA = 60;

    //https://www.geogebra.org/calculator/w7spyufu
    const std::vector<glm::vec2> POLYGON3 = consistentStartOfCircularList<2>({{100, 100}, {300, 100}, {300, 300}, {100, 300}});
    const std::vector<glm::vec2> POLYGON4 = consistentStartOfCircularList<2>({{50, 150}, {200, 50}, {350, 150}, {350, 300}, {250, 300}, {200, 250}, {150, 350}, {100, 250}, {100, 200}});
    const std::vector<glm::vec2> INTERSECTION_3_4 = consistentStartOfCircularList<2>({{300, 300}, {250, 300}, {200, 250}, {175, 300}, {125, 300}, {100, 250}, {100, 116.66666666666f}, {125, 100}, {275, 100}, {300, 116.66666666666f}});
    const float POLYGON3_AREA = 40000;
    const float POLYGON4_AREA = 52500;

    //https://www.geogebra.org/calculator/gfvbxc7m
    const std::vector<glm::vec2> POLYGON5 = consistentStartOfCircularList<2>({{10.0, 10.0}, {10.0, 100.0}, {100.0, 100.0}, {100.0, 10.0}});
    const std::vector<glm::vec2> POLYGON6 = consistentStartOfCircularList<2>({{20, 150.0}, {90, 150.0}, {80, 50.0}, {90, 115.0}, {80, 15}, {60, 120.0}, {40, 50}});
    const std::vector<glm::vec2> INTERSECTION_5_6_A = consistentStartOfCircularList<2>({{30, 100}, {54.2857142857143f, 100}, {40, 50}});
    const std::vector<glm::vec2> INTERSECTION_5_6_B = consistentStartOfCircularList<2>({{63.8095238095238f, 100}, {85, 100}, {80, 50}, {87.6923076923077f, 100}, {88.5f, 100}, {80, 15}});
    const std::vector<glm::vec2> UNION_5_6 = consistentStartOfCircularList<2>({{20, 150}, {90, 150}, {85, 100}, {87.6923076923077f, 100}, {90, 115}, {88.5f, 100}, {100, 100}, {100, 10}, {10, 10}, {10, 100}, {30, 100}});
    const std::vector<glm::vec2> UNION_5_6_HOLE = consistentStartOfCircularList<2>({{54.2857142857143f, 100}, {63.8095238095238f, 100}, {60, 120}});
    const std::vector<glm::vec2> DIFFERENCE_5_6_A = consistentStartOfCircularList<2>({{30, 100}, {10, 100}, {10, 10}, {100, 10}, {100, 100}, {88.5, 100}, {80, 15}, {63.8095238095238f, 100}, {54.2857142857143f, 100}, {40, 50}});
    const std::vector<glm::vec2> DIFFERENCE_5_6_B = consistentStartOfCircularList<2>({{85, 100}, {87.6923076923077f, 100}, {80, 50}});
    const float POLYGON5_AREA = -8100;
    const float POLYGON6_AREA = -4625;

    //https://www.geogebra.org/calculator/udmwxbte
    const std::vector<glm::vec2> POLYGON7 = consistentStartOfCircularList<2>({{-1, -1}, {2, -3}, {5, -3}, {10, -2}, {11, -1}, {11, 0}, {10, 5}, {7, 8}, {6, 8}, {3, 7}, {1, 5}});

    TEST_CASE("geometry::sutherlandHogmanPolygonClipping1") {
        const std::vector<glm::vec2> clipPolygon = reorderCircularList(POLYGON1, GENERATE(range(0, (int)POLYGON1.size())));
        const std::vector<glm::vec2> subjectPolygon = reorderCircularList(POLYGON2, GENERATE(range(0, (int)POLYGON2.size())));
        auto clipped = geometry::sutherlandHogmanPolygonClipping(subjectPolygon, clipPolygon);
        CHECK_VEC_VECTOR(INTERSECTION_1_2_CCW, consistentStartOfCircularList(clipped));
    }

    TEST_CASE("geometry::sutherlandHogmanPolygonClipping2") {
        const std::vector<glm::vec2> clipPolygon = reorderCircularList(POLYGON3, GENERATE(range(0, (int)POLYGON3.size())));
        const std::vector<glm::vec2> subjectPolygon = reorderCircularList(POLYGON4, GENERATE(range(0, (int)POLYGON4.size())));
        auto clipped = geometry::sutherlandHogmanPolygonClipping(subjectPolygon, clipPolygon);
        CHECK_VEC_VECTOR(INTERSECTION_3_4, consistentStartOfCircularList(clipped));
    }

    TEST_CASE("geometry::getSignedPolygonArea") {
        CHECK(POLYGON1_AREA == Approx(geometry::getSignedPolygonArea(POLYGON1)));
        CHECK(POLYGON2_AREA == Approx(geometry::getSignedPolygonArea(POLYGON2)));
        CHECK(POLYGON3_AREA == Approx(geometry::getSignedPolygonArea(POLYGON3)));
        CHECK(POLYGON4_AREA == Approx(geometry::getSignedPolygonArea(POLYGON4)));
        CHECK(POLYGON5_AREA == Approx(geometry::getSignedPolygonArea(POLYGON5)));
        CHECK(POLYGON6_AREA == Approx(geometry::getSignedPolygonArea(POLYGON6)));
    }

    TEST_CASE("polyclip1") {
        polyclip::Polygon polygon1(reorderCircularList(POLYGON5, GENERATE(range(0, (int)POLYGON5.size()))));
        polyclip::Polygon polygon2(reorderCircularList(POLYGON6, GENERATE(range(0, (int)POLYGON6.size()))));

        polyclip::PolygonOperation::detectIntersection(polygon1, polygon2);

        SECTION("intersection") {
            auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(polygon1, polygon2, polyclip::MARK_INTERSECTION);
            REQUIRE(normalIntersection);
            std::vector<std::vector<glm::vec2>> results = polyclip::PolygonOperation::extractIntersectionResults(polygon1);
            REQUIRE(results.size() == 2);
            if (results[0].size() == INTERSECTION_5_6_B.size()) {
                std::swap(results[0], results[1]);
            }
            CHECK_VEC_VECTOR(consistentStartOfCircularList(results[0]), INTERSECTION_5_6_A);
            CHECK_VEC_VECTOR(consistentStartOfCircularList(results[1]), INTERSECTION_5_6_B);
        }

        SECTION("union") {
            auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(polygon1, polygon2, polyclip::MARK_UNION);
            REQUIRE(normalIntersection);
            std::vector<std::vector<glm::vec2>> results = polyclip::PolygonOperation::extractUnionResults(polygon1);
            REQUIRE(results.size() == 2);
            if (results[0].size() == UNION_5_6_HOLE.size()) {
                std::swap(results[0], results[1]);
            }
            std::reverse(results[0].begin(), results[0].end());//don't know why this is needed
            CHECK_VEC_VECTOR(consistentStartOfCircularList(results[0]), UNION_5_6);
            CHECK_VEC_VECTOR(consistentStartOfCircularList(results[1]), UNION_5_6_HOLE);
        }

        SECTION("differentiate") {
            auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(polygon1, polygon2, polyclip::MARK_DIFFERENTIATE);
            REQUIRE(normalIntersection);
            std::vector<std::vector<glm::vec2>> results = polyclip::PolygonOperation::extractDifferentiateResults(polygon1);
            REQUIRE(results.size() == 2);
            if (results[0].size() == DIFFERENCE_5_6_B.size()) {
                std::swap(results[0], results[1]);
            }
            CHECK_VEC_VECTOR(consistentStartOfCircularList(results[0]), DIFFERENCE_5_6_A);
            CHECK_VEC_VECTOR(consistentStartOfCircularList(results[1]), DIFFERENCE_5_6_B);
        }
    }

    TEST_CASE("polyclip2") {
        polyclip::Polygon polygon1(consistentStartOfCircularList(POLYGON1));
        polyclip::Polygon polygon2(consistentStartOfCircularList(POLYGON2));

        polyclip::PolygonOperation::detectIntersection(polygon1, polygon2);
        auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(polygon1, polygon2, polyclip::MARK_INTERSECTION);
        REQUIRE(normalIntersection);

        std::vector<std::vector<glm::vec2>> results = polyclip::PolygonOperation::extractIntersectionResults(polygon1);
        REQUIRE(results.size() == 1);

        CHECK_VEC_VECTOR(consistentStartOfCircularList(results[0]), INTERSECTION_1_2_CCW);
    }

    TEST_CASE("polyclip3") {
        //these polygons are far away from each other
        polyclip::Polygon polygon1(reorderCircularList(POLYGON1, GENERATE(range(0, (int)POLYGON1.size()))));
        polyclip::Polygon polygon2(reorderCircularList(POLYGON3, GENERATE(range(0, (int)POLYGON3.size()))));

        SECTION("intersection") {
            polyclip::PolygonOperation::detectIntersection(polygon1, polygon2);
            auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(polygon1, polygon2, polyclip::MARK_INTERSECTION);
            REQUIRE_FALSE(normalIntersection);

            REQUIRE(possibleResult.empty());
        }

        SECTION("union") {
            polyclip::PolygonOperation::detectIntersection(polygon1, polygon2);
            auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(polygon1, polygon2, polyclip::MARK_UNION);
            REQUIRE_FALSE(normalIntersection);
            REQUIRE(possibleResult.size() == 2);

            if (possibleResult[0].size() == POLYGON3.size()) {
                std::swap(possibleResult[0], possibleResult[1]);
            }

            CHECK_VEC_VECTOR(consistentStartOfCircularList(possibleResult[0]), POLYGON1);
            CHECK_VEC_VECTOR(consistentStartOfCircularList(possibleResult[1]), POLYGON3);
        }

        SECTION("difference") {
            polyclip::PolygonOperation::detectIntersection(polygon1, polygon2);
            auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(polygon1, polygon2, polyclip::MARK_DIFFERENTIATE);
            REQUIRE_FALSE(normalIntersection);
            REQUIRE(possibleResult.size() == 1);

            CHECK_VEC_VECTOR(consistentStartOfCircularList(possibleResult[0]), POLYGON1);
        }
    }

    TEST_CASE("geometry::joinTrianglesToPolygon") {
        auto polygon = GENERATE(POLYGON1, POLYGON2, POLYGON7);

        std::vector<std::vector<glm::vec2>> polyWrapper;
        polyWrapper.push_back(polygon);
        const auto triangleIndices = mapbox::earcut(polyWrapper);

        std::vector<std::array<glm::vec2, 3>> triangles;
        for (std::size_t i = 0; i < triangleIndices.size(); i += 3) {
            triangles.push_back({
                    glm::vec2(polygon[triangleIndices[i]]),
                    glm::vec2(polygon[triangleIndices[i + 1]]),
                    glm::vec2(polygon[triangleIndices[i + 2]]),
            });
        }

        const auto joined = geometry::joinTrianglesToPolygon(triangles);
        REQUIRE(joined.size() == 1);
        CHECK_VEC_VECTOR(polygon, consistentStartOfCircularList(joined[0]));
    }
}