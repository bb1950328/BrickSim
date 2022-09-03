#include "geometry.h"
#include <array>
#include <cmath>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/normal.hpp>
#include <vector>

namespace bricksim::geometry {
    float calculateDistanceOfPointToLine(const glm::vec2& line_start, const glm::vec2& line_end, const glm::vec2& point) {
        //https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
        float numerator = std::abs((line_end.x - line_start.x) * (line_start.y - point.y) - (line_start.x - point.x) * (line_end.y - line_start.y));
        float denominator = std::sqrt(std::pow(line_end.x - line_start.x, 2.0f) + std::pow(line_end.y - line_start.y, 2.0f));
        return numerator / denominator;
    }

    float calculateDistanceOfPointToLine(const glm::usvec2& line_start, const glm::usvec2& line_end, const glm::usvec2& point) {
        int numerator = std::abs((line_end.x - line_start.x) * (line_start.y - point.y) - (line_start.x - point.x) * (line_end.y - line_start.y));
        float denominator = std::sqrt(std::pow(line_end.x - line_start.x, 2.0f) + std::pow(line_end.y - line_start.y, 2.0f));
        return numerator / denominator;
    }

    NormalProjectionResult normalProjectionOnLineClamped(const glm::vec2& lineStart, const glm::vec2& lineEnd, const glm::vec2& point) {
        //https://stackoverflow.com/a/47366970/8733066
        //             point
        //             +
        //          /  |
        //       /     |
        // start-------⦝----- end
        //             ↑
        //         nearestPointOnLine
        //
        // projection is from start to nearestPointOnLine
        NormalProjectionResult result{};
        glm::vec2 line = lineEnd - lineStart;
        result.lineLength = glm::length(line);
        glm::vec2 lineUnit = line / result.lineLength;
        glm::vec2 startToPoint = point - lineStart;
        result.projectionLength = glm::dot(startToPoint, lineUnit);

        if (result.projectionLength > result.lineLength) {
            result.nearestPointOnLine = lineEnd;
            result.projectionLength = result.lineLength;
            result.projection = line;
        } else if (result.projectionLength < 0.0f) {
            result.nearestPointOnLine = lineStart;
            result.projectionLength = 0.0f;
            result.projection = {0, 0};
        } else {
            result.projection = lineUnit * result.projectionLength;
            result.nearestPointOnLine = lineStart + result.projection;
        }
        result.distancePointToLine = glm::length(point - result.nearestPointOnLine);

        return result;
    }

    void gaussianElimination(std::array<float, 12>& matrix) {
        constexpr auto cols = 4;
        constexpr auto rows = 3;
        for (int i = 0; i < cols - 1; i++) {
            for (int j = i; j < rows; j++) {
                if (matrix[i + j * cols] != 0) {
                    if (i != j) {
                        for (int k = i; k < cols; k++) {
                            std::swap(matrix[k + i * cols], matrix[k + j * cols]);
                        }
                    }

                    j = i;

                    for (int v = 0; v < rows; v++) {
                        if (v == j) {
                            continue;
                        } else {
                            float factor = matrix[i + v * cols] / matrix[i + j * cols];
                            matrix[i + v * cols] = 0;

                            for (int u = i + 1; u < cols; u++) {
                                matrix[u + v * cols] -= factor * matrix[u + j * cols];
                                matrix[u + j * cols] /= matrix[i + j * cols];
                            }
                            matrix[i + j * cols] = 1;
                        }
                    }
                    break;
                }
            }
        }
    }

    ClosestLineBetweenTwoRaysResult closestLineBetweenTwoRays(const Ray3& a, const Ray3& b) {
        const auto p1 = a.origin;
        const auto d1 = glm::normalize(a.direction);
        const auto p2 = b.origin;
        const auto d2 = glm::normalize(b.direction);

        if (glm::all(glm::epsilonEqual(d1, d2, 0.001f))) {
            //rays are parallel -> we can do a normal projection
            //there are infinite solutions in this case so we set pointOnA to startA
            glm::vec3 startToStart = p1 - p2;
            float x = (glm::dot(d2, startToStart) / glm::length2(d2));
            auto pointOnB = p2 + x * d2;
            return {
                    .pointOnA = p1,
                    .pointOnB = pointOnB,
                    .distanceToPointA = 0,
                    .distanceToPointB = x,
                    .distanceBetweenPoints = glm::length(p1 - pointOnB),
            };
        } else {
            //https://math.stackexchange.com/a/1702955
            const auto n = glm::cross(d1, d2);
            const auto n1 = glm::cross(d1, n);
            const auto n2 = glm::cross(d2, n);
            const auto factor1 = glm::dot((p2 - p1), n2) / glm::dot(d1, n2);
            const auto factor2 = glm::dot((p1 - p2), n1) / glm::dot(d2, n1);
            const auto c1 = p1 + factor1 * d1;
            const auto c2 = p2 + factor2 * d2;

            return {
                    .pointOnA = c1,
                    .pointOnB = c2,
                    .distanceToPointA = factor1,
                    .distanceToPointB = factor2,
                    .distanceBetweenPoints = glm::length(c1 - c2),
            };
        }
    }

    std::optional<glm::vec3> rayPlaneIntersection(const Ray3& ray, const Ray3& planeNormal) {
        //https://math.stackexchange.com/a/2121529/945069
        const auto normalizedRayDir = glm::normalize(ray.direction);
        const auto normalizedPlaneDir = glm::normalize(planeNormal.direction);
        const float rayScale = glm::dot(normalizedPlaneDir, (planeNormal.origin - ray.origin)) / glm::dot(normalizedPlaneDir, normalizedRayDir);
        if (rayScale < 0) {
            return {};
        } else {
            return ray.origin + rayScale * normalizedRayDir;
        }
    }

    float getAngleBetweenThreePointsUnsigned(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        const auto ab = b - a;
        const auto bc = b - c;
        return std::acos(glm::dot(ab, bc) / (glm::length(ab) * glm::length(bc)));
    }

    float getAngleBetweenThreePointsSigned(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& planeNormal) {
        const auto ab = b - a;
        const auto cb = b - c;
        return std::atan2(glm::dot(glm::cross(ab, cb), glm::normalize(planeNormal)), glm::dot(ab, cb));
    }

    glm::quat quaternionRotationFromOneVectorToAnother(const glm::vec3& v1, const glm::vec3& v2) {
        //https://stackoverflow.com/a/11741520/8733066 second solution
        const auto v1n = glm::normalize(v1);
        const auto v2n = glm::normalize(v2);
        float k_cos_theta = glm::dot(v1n, v2n);
        float k = glm::sqrt(glm::length2(v1n) * glm::length2(v2n));
        if (k_cos_theta / k == -1) {
            // 180° rotation around any vector
            return {0, glm::normalize(getAnyPerpendicularVector(v1n))};
        }
        return glm::normalize(glm::quat(k_cos_theta + k, glm::cross(v1n, v2n)));
    }

    glm::vec3 getAnyPerpendicularVector(const glm::vec3& v) {
        return {1.f, 1.f, (-v.x - v.y) / v.z};
    }

    float getDistanceBetweenPointAndPlane(const Ray3& planeNormal, const glm::vec3& point) {
        //basically a perpendicular projection on the normal line
        return glm::dot(point - planeNormal.origin, glm::normalize(planeNormal.direction));
    }

    std::optional<glm::vec3> linePlaneIntersection(const glm::vec3& lineP0, const glm::vec3& lineP1, const Ray3& planeNormal) {
        auto u = lineP1 - lineP0;
        const auto dot = glm::dot(planeNormal.direction, u);
        if (glm::abs(dot) > 1e-6f) {
            const auto w = lineP0 - planeNormal.origin;
            const auto fac = -glm::dot(planeNormal.direction, w) / dot;
            u *= fac;
            return lineP0 + u;
        } else {
            return {};
        }
    }

    std::optional<glm::vec3> segmentPlaneIntersection(const glm::vec3& lineP0, const glm::vec3& lineP1, const Ray3& planeNormal) {
        auto u = lineP1 - lineP0;
        const auto dot = glm::dot(planeNormal.direction, u);
        if (glm::abs(dot) > 1e-6f) {
            const auto w = lineP0 - planeNormal.origin;
            const auto fac = -glm::dot(planeNormal.direction, w) / dot;
            if (fac > 1 || fac < 0) {
                return {};
            }
            return lineP0 + (u * fac);
        } else {
            return {};
        }
    }

    bool isPointInTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& point) {
        const auto a = p1 - point;
        const auto b = p2 - point;
        const auto c = p3 - point;

        const auto u = glm::cross(b, c);
        const auto v = glm::cross(c, a);
        const auto w = glm::cross(a, b);

        return glm::dot(u, v) >= 0.f && glm::dot(u, w) >= 0.f;
    }

    bool sutherlandHogmanInside(const glm::vec2& p, const glm::vec2& p1, const glm::vec2& p2) {
        return (p2.y - p1.y) * p.x + (p1.x - p2.x) * p.y + (p2.x * p1.y - p1.x * p2.y) < 0;
    }

    glm::vec2 sutherlandHogmanIntersection(const glm::vec2& cp1, const glm::vec2& cp2, const glm::vec2& s, const glm::vec2& e) {
        const glm::vec2 dc = {cp1.x - cp2.x, cp1.y - cp2.y};
        const glm::vec2 dp = {s.x - e.x, s.y - e.y};

        const float n1 = cp1.x * cp2.y - cp1.y * cp2.x;
        const float n2 = s.x * e.y - s.y * e.x;
        const float n3 = 1.0f / (dc.x * dp.y - dc.y * dp.x);

        return {(n1 * dp.x - n2 * dc.x) * n3, (n1 * dp.y - n2 * dc.y) * n3};
    }

    std::vector<glm::vec2> sutherlandHogmanPolygonClipping(const std::vector<glm::vec2>& subjectPolygon, const std::vector<glm::vec2>& clipPolygon) {
        glm::vec2 cp1, cp2, s, e;
        std::vector<glm::vec2> inputPolygon;
        std::vector<glm::vec2> outputPolygon = subjectPolygon;
        cp1 = clipPolygon[clipPolygon.size() - 1];
        for (const auto& clipVertex: clipPolygon) {
            cp2 = clipVertex;
            inputPolygon = outputPolygon;
            outputPolygon.clear();
            s = inputPolygon[inputPolygon.size() - 1];

            for (const auto& subjectVertex: inputPolygon) {
                e = subjectVertex;
                const bool sInside = sutherlandHogmanInside(s, cp1, cp2);
                const bool eInside = sutherlandHogmanInside(e, cp1, cp2);
                if (eInside) {
                    if (!sInside) {
                        outputPolygon.push_back(sutherlandHogmanIntersection(cp1, cp2, s, e));
                    }
                    outputPolygon.push_back(e);
                } else if (sInside) {
                    outputPolygon.push_back(sutherlandHogmanIntersection(cp1, cp2, s, e));
                }
                s = e;
            }
            cp1 = cp2;
        }
        return outputPolygon;
    }

    bool is2dPolygonClockwise(const std::vector<glm::vec2>& polygon) {
        float signedArea = getSignedPolygonArea(polygon);
        return signedArea < 0;
    }

    float getSignedPolygonArea(const std::vector<glm::vec2>& polygon) {
        float signedArea = 0;
        for (size_t i1 = 0, i2 = 1; i1 < polygon.size(); ++i1, i2 = (i1 + 1) % polygon.size()) {
            signedArea += (polygon[i1].x * polygon[i2].y - polygon[i2].x * polygon[i1].y);
        }
        signedArea *= 0.5;
        return signedArea;
    }

    Plane3dTo2dConverter::Plane3dTo2dConverter(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) :
        origin(a), xDir(glm::normalize(b - a)) {
        planeNormal = glm::triangleNormal(a, b, c);
        yDir = glm::rotate(glm::mat4(1.f), static_cast<float>(M_PI_2), planeNormal) * glm::vec4(xDir, 0.f);
    }

    glm::vec2 Plane3dTo2dConverter::convert3dTo2d(const glm::vec3& pointOnPlane) {
        const auto& r_P = pointOnPlane;
        const auto& n = planeNormal;
        const auto& r_O = origin;
        const auto& e_1 = xDir;
        const auto& e_2 = yDir;

        const auto s = glm::dot(n, r_P - r_O);
        const auto t_1 = glm::dot(e_1, r_P - r_O);
        const auto t_2 = glm::dot(e_2, r_P - r_O);
        return {t_1, t_2};
    }

    glm::vec3 Plane3dTo2dConverter::convert2dTo3d(const glm::vec2& coord) {
        return origin + coord.x * xDir + coord.y * yDir;
    }

    glm::vec3 triangleCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
        return (p1 + p2 + p3) / 3.0f;
    }

    glm::vec3 quadrilateralCentroid(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4) {
        return (p1 + p2 + p3 + p4) / 4.0f;
    }

    bool doesTransformationInverseWindingOrder(const glm::mat4& transformation) {
        glm::vec3 vec1 = transformation[0];
        glm::vec3 vec2 = transformation[1];
        glm::vec3 vec3 = transformation[2];
        glm::vec3 cross = glm::cross(vec1, vec2);
        return glm::dot(cross, vec3) < 0.0f;
    }

    std::vector<std::vector<glm::vec3>> splitPolygonByPlane(const std::vector<glm::vec3>& originalPoly, const Ray3& plane) {
        std::vector<std::vector<glm::vec3>> polygonParts;
        std::vector<std::tuple<size_t, glm::vec3, float>> intersections;
        glm::vec3 firstIntersectionPoint;
        for (size_t i1 = 0, i2 = 1; i1 < originalPoly.size(); ++i1, i2 = (i1 + 1) % originalPoly.size()) {
            const auto& ep1 = originalPoly[i1];
            const auto& ep2 = originalPoly[i2];
            const auto isOpt = geometry::segmentPlaneIntersection(ep1, ep2, plane);
            if (isOpt.has_value()) {
                if (intersections.empty()) {
                    firstIntersectionPoint = isOpt.value();
                }
                intersections.emplace_back(i1, isOpt.value(), glm::length2(firstIntersectionPoint - isOpt.value()));
            }
        }
        assert(intersections.size() % 2 == 0);

        if (intersections.empty()) {
            polygonParts.push_back(originalPoly);
        } else {
            glm::vec3 endPoint = std::get<1>(*std::max_element(intersections.begin(), intersections.end(), [](const auto& a, const auto& b) { return std::get<2>(a) < std::get<2>(b); }));
            for (auto& item: intersections) {
                std::get<2>(item) = glm::length2(std::get<1>(item) - endPoint);
            }
            std::sort(intersections.begin(), intersections.end(), [](const auto& a, const auto& b) { return std::get<2>(a) < std::get<2>(b); });
            std::vector<size_t> startPoints;
            startPoints.push_back(0);
            uoset_t<size_t> visitedPoints;
            while (!startPoints.empty()) {
                std::vector<glm::vec3> part;
                size_t i = startPoints.back();
                startPoints.pop_back();
                if (visitedPoints.contains(i)) {
                    continue;
                }
                size_t iBegin = i;
                do {
                    visitedPoints.insert(i);
                    part.push_back(originalPoly[i]);
                    auto it = std::find_if(intersections.begin(), intersections.end(), [&i](const auto& is) { return std::get<0>(is) == i; });
                    if (it != intersections.end()) {
                        part.push_back(std::get<1>(*it));
                        if (std::distance(intersections.begin(), it) % 2 == 0) {
                            ++it;
                        } else {
                            --it;
                        }
                        part.push_back(std::get<1>(*it));
                        size_t possibleStartPoint = (i + 1) % originalPoly.size();
                        if (!visitedPoints.contains(possibleStartPoint)) {
                            startPoints.push_back(possibleStartPoint);
                        }
                        i = std::get<0>(*it);
                    }
                    i = (i + 1) % originalPoly.size();
                } while (i != iBegin);
                polygonParts.push_back(part);
            }
        }
        return polygonParts;
    }

    bool doesTransformationLeaveAxisParallels(const glm::mat4& transformation) {
        return doesTransformationLeaveAxisParallels(glm::quat_cast(transformation));
    }

    bool doesTransformationLeaveAxisParallels(const glm::quat& quaternion) {
        const auto eulerAngles = glm::eulerAngles(glm::normalize(quaternion));
        constexpr float epsilon = 0.0001;
        bool res = true;
        for (int i = 0; i < 3; ++i) {
            res &= (std::fabs(eulerAngles[i]) < epsilon
                    || std::fabs(eulerAngles[i] - 1.f) < epsilon
                    || std::fabs(eulerAngles[i] + 1.f) < epsilon);
        }
        return res;
    }
    std::vector<glm::vec2> convertPolygonWithHoleToC(const std::vector<glm::vec2>& outerPoly, const std::vector<glm::vec2>& holePoly) {
        size_t maxXinHoleIndex = 0;
        for (int i = 1; i < holePoly.size(); ++i) {
            if (holePoly[maxXinHoleIndex].x < holePoly[i].x) {
                maxXinHoleIndex = i;
            }
        }
        const auto maxXinHole = holePoly[maxXinHoleIndex].x;
        size_t nextLargerXinOuter = 0;
        for (int i = 1; i < outerPoly.size(); ++i) {
            if (outerPoly[nextLargerXinOuter].x < maxXinHole || (outerPoly[nextLargerXinOuter].x > outerPoly[i].x && outerPoly[i].x >= maxXinHole)) {
                nextLargerXinOuter = i;
            }
        }
        std::vector<glm::vec2> result;
        result.insert(result.end(), outerPoly.begin(), outerPoly.begin() + nextLargerXinOuter + 1);
        result.insert(result.end(), holePoly.begin() + maxXinHoleIndex, holePoly.end());
        result.insert(result.end(), holePoly.begin(), holePoly.begin() + maxXinHoleIndex + 1);
        result.insert(result.end(), outerPoly.begin() + nextLargerXinOuter, outerPoly.end());
        return result;
    }
}
