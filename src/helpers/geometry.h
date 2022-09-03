#pragma once

#include "../types.h"
#include "ray.h"
#include "util.h"
#include <glm/glm.hpp>
#include <optional>

namespace bricksim::geometry {
    glm::vec3 triangleCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
    glm::vec3 quadrilateralCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4);
    bool doesTransformationInverseWindingOrder(const glm::mat4& transformation);
    float calculateDistanceOfPointToLine(const glm::vec2& line_start, const glm::vec2& line_end, const glm::vec2& point);
    float calculateDistanceOfPointToLine(const glm::usvec2& line_start, const glm::usvec2& line_end, const glm::usvec2& point);
    struct NormalProjectionResult {
        glm::vec2 nearestPointOnLine;
        glm::vec2 projection;
        float projectionLength;
        float distancePointToLine;
        float lineLength;
    };
    NormalProjectionResult normalProjectionOnLineClamped(const glm::vec2& lineStart, const glm::vec2& lineEnd, const glm::vec2& point);
    struct ClosestLineBetweenTwoRaysResult {
        glm::vec3 pointOnA;
        glm::vec3 pointOnB;
        float distanceToPointA;
        float distanceToPointB;
        float distanceBetweenPoints;
    };
    ClosestLineBetweenTwoRaysResult closestLineBetweenTwoRays(const Ray3& a, const Ray3& b);

    std::optional<glm::vec3> rayPlaneIntersection(const Ray3& ray, const Ray3& planeNormal);
    std::optional<glm::vec3> linePlaneIntersection(const glm::vec3& lineP0, const glm::vec3& lineP1, const Ray3& planeNormal);
    std::optional<glm::vec3> segmentPlaneIntersection(const glm::vec3& lineP0, const glm::vec3& lineP1, const Ray3& planeNormal);

    glm::quat quaternionRotationFromOneVectorToAnother(const glm::vec3& v1, const glm::vec3& v2);
    glm::vec3 getAnyPerpendicularVector(const glm::vec3& v);

    float getAngleBetweenThreePointsUnsigned(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
    float getAngleBetweenThreePointsSigned(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& planeNormal);

    float getDistanceBetweenPointAndPlane(const Ray3& planeNormal, const glm::vec3& point);

    bool isPointInTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& point);

    class Plane3dTo2dConverter {
        glm::vec3 origin, xDir, yDir, planeNormal;

    public:
        Plane3dTo2dConverter(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
        glm::vec2 convert3dTo2d(const glm::vec3& pointOnPlane);
        glm::vec3 convert2dTo3d(const glm::vec2& coord);
    };

    /**
     * polygons are in CCW order
     */
    std::vector<glm::vec2> sutherlandHogmanPolygonClipping(const std::vector<glm::vec2>& subjectPolygon, const std::vector<glm::vec2>& clipPolygon);

    float getSignedPolygonArea(const std::vector<glm::vec2>& polygon);
    bool is2dPolygonClockwise(const std::vector<glm::vec2>& polygon);

    template<glm::length_t L>
    int findPointInPolygonEpsilon(std::vector<glm::vec<L, float, glm::defaultp>> poly, glm::vec<L, float, glm::defaultp> point) {
        for (int i = 0; i < poly.size(); ++i) {
            if (util::vectorSum(glm::abs(poly[i] - point)) < 0.0003f) {
                return i;
            }
        }
        return -1;
    }

    /**
     * WARNING: this function is horribly inefficient because it's brute force
     */
    template<glm::length_t L>
    std::vector<std::vector<glm::vec<L, float, glm::defaultp>>> joinTrianglesToPolygon(std::vector<std::array<glm::vec<L, float, glm::defaultp>, 3>> triangles) {
        static_assert(L == 2 || L == 3, "this only works for 2D and 3D coordinates");
        typedef glm::vec<L, float, glm::defaultp> point;
        std::vector<std::vector<point>> result;
        while (!triangles.empty()) {
            std::vector<point> currentPoly(triangles.back().begin(), triangles.back().end());
            triangles.pop_back();
            bool foundOne;
            do {
                foundOne = false;
                for (auto tri = triangles.begin(); tri < triangles.end();) {
                    const auto i0 = findPointInPolygonEpsilon(currentPoly, (*tri)[0]);
                    const auto i1 = findPointInPolygonEpsilon(currentPoly, (*tri)[1]);
                    const auto i2 = findPointInPolygonEpsilon(currentPoly, (*tri)[2]);
                    point additionalCoord;
                    int ia, ib;
                    foundOne = true;
                    if (i0 >= 0 && i1 >= 0) {
                        additionalCoord = (*tri)[2];
                        ia = i0;
                        ib = i1;
                    } else if (i0 >= 0 && i2 >= 0) {
                        additionalCoord = (*tri)[1];
                        ia = i0;
                        ib = i2;
                    } else if (i1 >= 0 && i2 >= 0) {
                        additionalCoord = (*tri)[0];
                        ia = i1;
                        ib = i2;
                    } else {
                        foundOne = false;
                    }
                    if (foundOne && abs(ia - ib) == 1) {
                        currentPoly.insert(currentPoly.begin() + std::max(ia, ib), additionalCoord);
                        tri = triangles.erase(tri);
                        break;
                    } else if (foundOne && std::min(ia, ib) == 0 && std::max(ia, ib) == currentPoly.size() - 1) {
                        currentPoly.push_back(additionalCoord);
                        tri = triangles.erase(tri);
                        break;
                    } else {
                        foundOne = false;
                        ++tri;
                    }
                }
            } while (foundOne);
            if constexpr (L == 2) {
                if (is2dPolygonClockwise(currentPoly)) {
                    std::reverse(currentPoly.begin(), currentPoly.end());
                }
            }
            result.push_back(currentPoly);
        }
        return result;
    }

    std::vector<std::vector<glm::vec3>> splitPolygonByPlane(const std::vector<glm::vec3>& originalPoly, const Ray3& plane);

    /**       converts this:                 into this:
     *     +-----------------+             +-----------------+
     *     |                 |             |                 |
     *     |    +------+     |             |    +------+     |
     *     |    |      |     |             |    |      |     |
     *     |    |      |     |             |    |      +-----+
     *     |    |      |     |             |    |      +-----+
     *     |    |      |     |             |    |      |     |
     *     |    +------+     |             |    +------+     |
     *     |                 |             |                 |
     *     +-----------------+             +-----------------+
     *
     * @param outerPoly
     * @param holePoly
     * @return
     */
    std::vector<glm::vec2> convertPolygonWithHoleToC(const std::vector<glm::vec2>& outerPoly, const std::vector<glm::vec2>& holePoly);

    /**
     * @return true if @param transformation only has rotation in 90° steps
     */
    bool doesTransformationLeaveAxisParallels(const glm::mat4& transformation);

    /**
     * @return true if @param quaternion only has rotation in 90° steps
     */
    bool doesTransformationLeaveAxisParallels(const glm::quat& quaternion);
}
