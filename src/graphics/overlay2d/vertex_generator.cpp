#include "vertex_generator.h"
#include "../../helpers/geometry.h"
namespace bricksim::overlay2d::vertex_generator {
    void generateVerticesForLine(std::vector<Vertex>::iterator& buffer, coord_t start, coord_t end, length_t width, color::RGB color, coord_t viewportSize) {
        // 1----------------------------------2
        // |                                  |
        // + start                        end +
        // |                                  |
        // 4----------------------------------3
        const auto startToEnd = end - start;
        const auto halfEdge = glm::normalize(glm::vec2(-startToEnd.y, startToEnd.x)) * (width / 2.f);
        const auto p1 = start - halfEdge;
        const auto p2 = start + halfEdge;
        const auto p3 = end + halfEdge;
        const auto p4 = end - halfEdge;
        return generateVerticesForQuad(buffer, p1, p2, p3, p4, color, viewportSize);
    }

    void generateVerticesForCCWTriangle(std::vector<Vertex>::iterator& buffer, coord_t p0, coord_t p1, coord_t p2, color::RGB color, coord_t viewportSize) {
        *buffer = {toNDC(p0, viewportSize), color.asGlmVector()};
        buffer++;
        *buffer = {toNDC(p1, viewportSize), color.asGlmVector()};
        buffer++;
        *buffer = {toNDC(p2, viewportSize), color.asGlmVector()};
        buffer++;
    }

    void generateVerticesForTriangle(std::vector<Vertex>::iterator& buffer, coord_t p0, coord_t p1, coord_t p2, color::RGB color, coord_t viewportSize) {
        if (geometry::is2dTriangleClockwise(p0, p1, p2)) {
            std::swap(p1, p2);
        }
        return generateVerticesForCCWTriangle(buffer, p0, p1, p2, color, viewportSize);
    }

    void generateVerticesForSquare(std::vector<Vertex>::iterator& buffer, coord_t center, length_t sideLength, color::RGB color, coord_t viewportSize) {
        const float halfSideLength = static_cast<float>(sideLength) / 2;
        const auto x = static_cast<float>(center.x);
        const auto y = static_cast<float>(center.y);
        glm::vec2 p1(x - halfSideLength, y + halfSideLength);
        glm::vec2 p2(x + halfSideLength, y + halfSideLength);
        glm::vec2 p3(x + halfSideLength, y - halfSideLength);
        glm::vec2 p4(x - halfSideLength, y - halfSideLength);
        return generateVerticesForQuad(buffer, p1, p2, p3, p4, color, viewportSize);
    }

    void generateVerticesForRegularPolygon(std::vector<Vertex>::iterator& buffer, coord_t center, length_t radius, short numEdges, color::RGB color,
                                           coord_t viewportSize) {
        float angleStep = 2 * static_cast<float>(M_PI) / static_cast<float>(numEdges);
        const auto cx = static_cast<float>(center.x);
        const auto cy = static_cast<float>(center.y);
        const auto fRadius = static_cast<float>(radius);
        const glm::vec2 p0 = toNDC(coord_t{fRadius + cx, cy}, viewportSize);
        glm::vec2 lastP = toNDC(glm::vec2{fRadius * std::cos(angleStep) + cx, fRadius * std::sin(angleStep) + cy}, viewportSize);
        for (short i = 2; i < numEdges; ++i) {
            const auto angle = angleStep * (i);
            glm::vec2 currentP = toNDC(glm::vec2{fRadius * std::cos(angle) + cx, fRadius * std::sin(angle) + cy}, viewportSize);

            *buffer = {p0, color.asGlmVector()};
            ++buffer;

            *buffer = {lastP, color.asGlmVector()};
            ++buffer;

            *buffer = {currentP, color.asGlmVector()};
            ++buffer;

            lastP = currentP;
        }
    }

    /**
         * p1 -- p2
         * |     |
         * p4 -- p3
         */
    void generateVerticesForQuad(std::vector<Vertex>::iterator& buffer,
                                 const glm::vec2& p1,
                                 const glm::vec2& p2,
                                 const glm::vec2& p3,
                                 const glm::vec2& p4,
                                 color::RGB color,
                                 coord_t viewportSize) {
        const auto p1ndc = toNDC(p1, viewportSize);
        const auto p2ndc = toNDC(p2, viewportSize);
        const auto p3ndc = toNDC(p3, viewportSize);
        const auto p4ndc = toNDC(p4, viewportSize);
        *buffer = {p1ndc, color.asGlmVector()};
        ++buffer;
        *buffer = {p4ndc, color.asGlmVector()};
        ++buffer;
        *buffer = {p3ndc, color.asGlmVector()};
        ++buffer;
        *buffer = {p3ndc, color.asGlmVector()};
        ++buffer;
        *buffer = {p2ndc, color.asGlmVector()};
        ++buffer;
        *buffer = {p1ndc, color.asGlmVector()};
        ++buffer;
    }

    std::pair<glm::vec2, glm::vec2> calculatePolyLineCornerPoints(length_t width, glm::vec2 point, glm::vec2 dirToLast, glm::vec2 dirToNext) {
        //https://math.stackexchange.com/a/1849845/945069
        const auto beta = geometry::getAngleBetweenTwoVectors(dirToLast, dirToNext);
        glm::vec2 uvSum;
        if (beta > M_PI - .01f) {
            //special case when beta is almost 180°
            uvSum = glm::normalize(glm::vec2(dirToLast.y, -dirToLast.x)) * (width / 2.f);
        } else {
            const auto uvLength = width / (2 * std::sin(beta));
            const auto u = glm::normalize(dirToLast) * uvLength;
            const auto v = glm::normalize(dirToNext) * uvLength;
            uvSum = u + v;
        }
        return std::make_pair(point + uvSum, point - uvSum);
    }

    void generateVerticesForPolyLine(std::vector<Vertex>::iterator& buffer, const std::vector<coord_t>& points, length_t width, color::RGB color, coord_t viewportSize) {
        std::pair<glm::vec2, glm::vec2> lastPoints{};
        for (size_t i = 0; i < points.size(); ++i) {
            const auto sToLast = i == 0
                                         ? (points[0] - points[1])
                                         : (points[i - 1] - points[i]);
            const auto sToNext = i == points.size() - 1
                                         ? (points[points.size() - 1] - points[points.size() - 2])
                                         : (points[i + 1] - points[i]);
            const auto fToLast = glm::normalize(glm::vec2(sToLast));
            const auto fToNext = glm::normalize(glm::vec2(sToNext));
            auto currentPoints = calculatePolyLineCornerPoints(width, points[i], fToLast, fToNext);
            if (i != 0) {
                // last              current
                // .first-------------.first
                // |                       |
                // .second-----------.second
                if (!geometry::is2dTriangleClockwise(lastPoints.second, lastPoints.first, currentPoints.first)) {
                    std::swap(lastPoints.first, lastPoints.second);
                }
                if (!geometry::is2dTriangleClockwise(lastPoints.first, currentPoints.first, currentPoints.second)) {
                    std::swap(currentPoints.first, currentPoints.second);
                }
                generateVerticesForQuad(buffer, lastPoints.first, currentPoints.first, currentPoints.second, lastPoints.second, color, viewportSize);
            }
            lastPoints = currentPoints;
        }
    }
}
