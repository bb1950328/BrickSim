#include "../../helpers/geometry.h"
#include "../testing_tools.h"
#include "catch2/catch.hpp"
#include <glm/gtc/epsilon.hpp>
#include <iostream>

namespace bricksim {
    TEST_CASE("geometry::doesTransformationInverseWindingOrder") {
        CHECK_FALSE(geometry::doesTransformationInverseWindingOrder(glm::mat4(1.0f)));
        CHECK_FALSE(geometry::doesTransformationInverseWindingOrder(glm::translate(glm::mat4(1.0f), {1, 2, 3})));
        CHECK_FALSE(geometry::doesTransformationInverseWindingOrder(glm::rotate(glm::mat4(1.0f), 1.0f, {1, 2, 3})));
        CHECK_FALSE(geometry::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {1, 2, 3})));
        CHECK(geometry::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, 2, 3})));
        CHECK_FALSE(geometry::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, -2, 3})));
        CHECK(geometry::doesTransformationInverseWindingOrder(glm::scale(glm::mat4(1.0f), {-1, -2, -3})));
    }

    TEST_CASE("geometry::calculateDistanceOfPointToLine") {
        CHECK(geometry::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {1.0f, 1.0f}, {0.0f, 1.0f}) == Approx(std::sqrt(2) / 2));
        CHECK(geometry::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {1.0f, 0.0f}, {1.0f, 1.0f}) == Approx(1));
        CHECK(geometry::calculateDistanceOfPointToLine(glm::vec2(0.0f, 0.0f), {0.0f, 1.0f}, {1.0f, 1.0f}) == Approx(1));
    }

    TEST_CASE("geometry::normalProjectionOnLineClamped 0") {
        auto result = geometry::normalProjectionOnLineClamped({0, 0}, {2, 0}, {1, 1});
        CHECK(result.distancePointToLine == Approx(1));
        CHECK(result.lineLength == Approx(2));
        CHECK(result.nearestPointOnLine == ApproxVec(glm::vec2(1, 0)));
        CHECK(result.projection == ApproxVec(glm::vec2(1, 0)));
        CHECK(result.projectionLength == Approx(1));
    }

    TEST_CASE("geometry::normalProjectionOnLineClamped 1") {
        auto result = geometry::normalProjectionOnLineClamped({1, 1}, {3, 3}, {1, 3});
        CHECK(result.distancePointToLine == Approx(sqrt(2)));
        CHECK(result.lineLength == Approx(2 * sqrt(2)));
        CHECK(result.nearestPointOnLine == ApproxVec(glm::vec2(2, 2)));
        CHECK(result.projection == ApproxVec(glm::vec2(1, 1)));
        CHECK(result.projectionLength == Approx(sqrt(2)));
    }

    TEST_CASE("geometry::normalProjectionOnLineClamped 2") {
        auto result = geometry::normalProjectionOnLineClamped({10, 20}, {14, 22}, {11, 23});
        CHECK(result.distancePointToLine == Approx(sqrt(2 * 2 + 1 * 1)));
        CHECK(result.lineLength == Approx(sqrt(4 * 4 + 2 * 2)));
        CHECK(result.nearestPointOnLine == ApproxVec(glm::vec2(12, 21)));
        CHECK(result.projection == ApproxVec(glm::vec2(2, 1)));
        CHECK(result.projectionLength == Approx(sqrt(2 * 2 + 1 * 1)));
    }

    void consistencyCheck(const Ray3& a, const Ray3& b, const geometry::ClosestLineBetweenTwoRaysResult& result) {
        CHECK(glm::length(result.pointOnA - result.pointOnB) == result.distanceBetweenPoints);
        CHECK(a.origin + glm::normalize(a.direction) * result.distanceToPointA == ApproxVec(result.pointOnA));
        CHECK(b.origin + glm::normalize(b.direction) * result.distanceToPointB == ApproxVec(result.pointOnB));
    }

    TEST_CASE("geometry::closestLineBetweenTwoRays0") {
        // https://keisan.casio.com/exec/system/1223531414
        const Ray3 a = {
                {-1, 2, 0},
                {2, 3, 1},
        };
        const Ray3 b = {
                {3, -4, 1},
                {1, 2, 1},
        };
        auto result = geometry::closestLineBetweenTwoRays(a, b);
        consistencyCheck(a, b, result);
        CHECK(result.distanceBetweenPoints == Approx(6.3508529610859));
        CHECK(result.pointOnA == ApproxVec(glm::vec3(5, 11, 3)));
        CHECK(result.pointOnB == ApproxVec(glm::vec3(26 / 3.f, 22 / 3.f, 20 / 3.f)));
    }

    TEST_CASE("geometry::closestLineBetweenTwoRays1") {
        //https://www.geogebra.org/calculator/ujvvt34f
        const Ray3 a = {
                {1, 2, 3},
                {2, 3, 4},
        };
        const Ray3 b = {
                {3, 2, 1},
                {4, 3, 2},
        };
        auto result = geometry::closestLineBetweenTwoRays(a, b);
        consistencyCheck(a, b, result);
        CHECK(result.distanceBetweenPoints == Approx(0.0f));
        CHECK(result.pointOnA == ApproxVec(glm::vec3(-1, -1, -1)));
        CHECK(result.pointOnB == ApproxVec(glm::vec3(-1, -1, -1)));
    }

    TEST_CASE("geometry::closestLineBetweenTwoRays2") {
        const Ray3 a = {
                {1, 2, 3},
                {2, 3, 4},
        };
        const Ray3 b = {
                {3, 2, 1},
                a.direction,
        };
        auto result = geometry::closestLineBetweenTwoRays(a, b);
        consistencyCheck(a, b, result);
        CHECK(result.distanceBetweenPoints == Approx(2.7291529568841f));
        CHECK(result.pointOnA == ApproxVec(a.origin));
        CHECK(result.pointOnB == ApproxVec(glm::vec3(3.27586198, 2.41379309, 1.5517242)));
    }

    TEST_CASE("geometry::closestLineBetweenTwoRays3") {
        const Ray3 a = {
                {9999, 9999, 50},
                {2, 3, 0},
        };
        const Ray3 b = {
                {9999, 9999, 150},
                {3, 2, 0},
        };
        auto result = geometry::closestLineBetweenTwoRays(a, b);
        consistencyCheck(a, b, result);
        CHECK(result.distanceBetweenPoints == Approx(100));
        CHECK(result.pointOnA == ApproxVec(glm::vec3(9999, 9999, 50)));
        CHECK(result.pointOnB == ApproxVec(glm::vec3(9999, 9999, 150)));
    }

    TEST_CASE("geometry::rayPlaneIntersection1") {
        //https://www.geogebra.org/calculator/gzzhkpdz
        const Ray3 ray = {{1, 2, 3}, {4, 5, 6}};
        const Ray3 plane = {{3, 2, 1}, {3, 4, 1}};
        auto result = geometry::rayPlaneIntersection(ray, plane);
        CHECK(result == ApproxVec(glm::vec3(1.421052631578947, 2.526315789473684, 3.631578947368421)));
    }

    TEST_CASE("geometry::rayPlaneIntersection2") {
        //https://www.geogebra.org/calculator/ukzyhnuk
        const Ray3 ray = {{1, 2, 3}, {4, -1, 2}};
        const Ray3 plane = {{12, 2, -2}, {3, -4, 1}};
        auto result = geometry::rayPlaneIntersection(ray, plane);
        CHECK(result == ApproxVec(glm::vec3(7.222222222222221, 0.444444444444445, 6.111111111111111)));
    }

    TEST_CASE("geometry::rayPlaneIntersection3") {
        //https://www.geogebra.org/m/vq6chbwb
        const Ray3 ray = {{1, 0, 0}, {4, -1, -2}};
        const Ray3 plane = {{12, 9, -2}, {3, 8, 1}};
        auto result = geometry::rayPlaneIntersection(ray, plane);
        CHECK(result == ApproxVec(glm::vec3(207.0, -51.5, -103.0)));
    }

    TEST_CASE("geometry::rayPlaneIntersection4") {
        //https://www.geogebra.org/calculator/bdjqrttr
        const Ray3 ray = {{1, 2, 3}, {-4, -1, 2}};
        const Ray3 plane = {{12, 2, -2}, {3, -4, 1}};
        auto result = geometry::rayPlaneIntersection(ray, plane);
        CHECK(!result.has_value());
    }

    TEST_CASE("geometry::rayPlaneIntersection5") {
        //ray is parallel to plane
        const Ray3 ray = {{0, 0, 10}, {1, 0, 0}};
        const Ray3 plane = {{0, 0, 0}, {0, 0, 1}};
        auto result = geometry::rayPlaneIntersection(ray, plane);
        CHECK(!result.has_value());
    }

    TEST_CASE("geometry::getAngleBetweenThreePointsUnsigned 1") {
        //https://www.geogebra.org/calculator/enejk3cx
        const float angle = geometry::getAngleBetweenThreePointsUnsigned({3, 5, 7}, {5, 2, 8}, {9, -2, 4});
        CHECK(glm::degrees(angle) == Approx(128.112926500986674));
    }

    TEST_CASE("geometry::getAngleBetweenThreePointsUnsigned 2") {
        const float angle = geometry::getAngleBetweenThreePointsUnsigned({1, 2, 3}, {3, 2, 1}, {1, 1, 1});
        CHECK(glm::degrees(angle) == Approx(50.76848));
    }

    TEST_CASE("geometry::getAngleBetweenThreePointsUnsigned 3") {
        const float angle = geometry::getAngleBetweenThreePointsUnsigned({1, 0, 0}, {0, 0, 0}, {0, 1, 0});
        CHECK(glm::degrees(angle) == Approx(90.0));
    }

    TEST_CASE("geometry::getAngleBetweenThreePointsUnsigned 4") {
        const float angle = geometry::getAngleBetweenThreePointsUnsigned({1, 0, 0}, {0, 0, 0}, {-5, 0, 0});
        CHECK(glm::degrees(angle) == Approx(180.0));
    }

    TEST_CASE("geometry::getDistanceBetweenPointAndPlane") {
        const glm::vec3 po(2, 3, 4);
        const glm::vec3 pn(3, 2, 1);
        //plane equation: 3x + 2y + z - 16 = 0, calculated with https://onlinemschool.com/math/assistance/cartesian_coordinate/plane/
        const glm::vec3 point(6, 4, 2);

        const float expectedDistance = 12.f / std::sqrt(14.f);//calculated with https://onlinemschool.com/math/assistance/cartesian_coordinate/p_plane/
        const float actualDistance = geometry::getDistanceBetweenPointAndPlane(Ray3(po, pn), point);
        CHECK(expectedDistance == Approx(actualDistance));
    }

    TEST_CASE("geometry::getDistanceBetweenPointAndPlane2") {
        const glm::vec3 po(0, 0, 0);
        const glm::vec3 pn(1, 1, 0);
        const glm::vec3 point(3, 1, 0);

        const float expectedDistance = 2.8284271247462f;
        const float actualDistance = geometry::getDistanceBetweenPointAndPlane(Ray3(po, pn), point);
        CHECK(expectedDistance == Approx(actualDistance));
    }

    TEST_CASE("geometry::getDistanceBetweenPointAndPlane3") {
        const glm::vec3 po(1, 2, 1);
        const glm::vec3 pn(1, -1, -1.5f);
        const glm::vec3 point(9, -4, 4);

        const float expectedDistance = 4.6081768756903f;
        const float actualDistance = geometry::getDistanceBetweenPointAndPlane(Ray3(po, pn), point);
        CHECK(expectedDistance == Approx(actualDistance));
    }

    TEST_CASE("geometry::is2dPolygonClockwise") {
        CHECK(geometry::is2dPolygonClockwise({{0, 0}, {0, 1}, {1, 0}}));
        CHECK_FALSE(geometry::is2dPolygonClockwise({{1, 1}, {0, 1}, {1, 0}}));
    }

    struct Plane3dTo2dConverterData {
        glm::vec3 point3d;
        glm::vec3 point3dOnPlane;
        glm::vec2 point2d;
    };

    TEST_CASE("geometry::Plane3dTo2dConverter 1") {
        //https://www.geogebra.org/calculator/npqaupp4
        glm::vec3 a(1, 3, 2);
        glm::vec3 b(2, 4, 1);
        glm::vec3 c(5, 2, 4);

        const auto data = GENERATE(
                Plane3dTo2dConverterData{
                        {3, 7, 3},
                        {3.4354838709677, 4.3870967741935, 0.8225806451613},
                        {2.8867513459481, 0.9532062476388}},
                Plane3dTo2dConverterData{
                        {6, 0, 1},
                        {5.5483870967742, 2.7096774193548, 3.258064516129},
                        {1.7320508075689, 4.3994134506406}},
                Plane3dTo2dConverterData{
                        {-4, 5, 6},
                        {-3.4032258064516, 1.4193548387097, 3.0161290322581},
                        {-4.0414518843274, -2.5663245128737}});

        geometry::Plane3dTo2dConverter planeConverter(a, b, c);
        CHECK(planeConverter.convert3dTo2d(data.point3d) == ApproxVec(data.point2d));
        CHECK(planeConverter.convert3dTo2d(data.point3dOnPlane) == ApproxVec(data.point2d));
        CHECK(planeConverter.convert2dTo3d(data.point2d) == ApproxVec(data.point3dOnPlane));
    }

    const std::vector<glm::vec3> POLYGON_TO_SPLIT{
            {2, 3, 5},
            {-3, 3, 3},
            {2, 3, 3},
            {-4, 3, 1},
            {-4, 3, -1},
            {8, 3, -1},
            {7, 3, 4},
    };

    TEST_CASE("geometry::splitPolygonByPlane 1") {
        //https://www.geogebra.org/calculator/hagcybt6
        glm::vec3 i1(-2.4444444444444, 3, 3.2222222222222);
        glm::vec3 i2(-2, 3, 3);
        glm::vec3 i3(-0.4, 3, 2.2);
        glm::vec3 i4(6, 3, -1);

        std::vector<std::vector<glm::vec3>> expectedResult{
                {POLYGON_TO_SPLIT[0], i1, i2, POLYGON_TO_SPLIT[2], i3, i4, POLYGON_TO_SPLIT[5], POLYGON_TO_SPLIT[6]},
                {i3, POLYGON_TO_SPLIT[3], POLYGON_TO_SPLIT[4], i4},
                {i1, POLYGON_TO_SPLIT[1], i2},
        };

        auto reorderedPoints = reorderCircularList(POLYGON_TO_SPLIT, GENERATE(range(0, (int)POLYGON_TO_SPLIT.size())));
        auto actualResult = geometry::splitPolygonByPlane(reorderedPoints, Ray3({2, 3, 1}, {1, 1, 2}));
        std::sort(actualResult.begin(), actualResult.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });

        REQUIRE(actualResult.size() == 3);
        CHECK_VEC_VECTOR(consistentStartOfCircularList(expectedResult[0]), consistentStartOfCircularList(actualResult[0]));
        CHECK_VEC_VECTOR(consistentStartOfCircularList(expectedResult[1]), consistentStartOfCircularList(actualResult[1]));
        CHECK_VEC_VECTOR(consistentStartOfCircularList(expectedResult[2]), consistentStartOfCircularList(actualResult[2]));
    }

    //todo tests for geometry::getAngleBetweenThreePointsSigned
}