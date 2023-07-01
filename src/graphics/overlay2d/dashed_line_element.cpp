#include "dashed_line_element.h"
#include "vertex_generator.h"
namespace bricksim::overlay2d {
    DashedLineElement::DashedLineElement(const std::vector<coord_t>& points, length_t spaceBetweenDashes, length_t width, const color::RGB& color) :
        BaseDashedLineElement(spaceBetweenDashes, width, color), points(points) {
        validatePoints();
    }
    bool DashedLineElement::isPointInside(coord_t point) {
        return false;//todo implement
    }
    unsigned int DashedLineElement::getVertexCount() {
        return points.empty() ? 0 : vertex_generator::getVertexCountForLine() * (points.size() - 1);
    }
    Vertex* DashedLineElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        if (points.empty()) {
            return firstVertexLocation;
        }
        Vertex* nextVertexLocation = firstVertexLocation;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i + 1];
            const glm::vec2 delta = p2 - p1;
            const glm::vec2 halfGap(delta * (spaceBetweenDashes / glm::length(delta) / 2.f));
            const coord_t lineStart = p1 + halfGap;
            const coord_t lineEnd = p2 - halfGap;
            nextVertexLocation = vertex_generator::generateVerticesForLine(nextVertexLocation, lineStart, lineEnd, width, color, viewportSize);
        }
        return nextVertexLocation;
    }
    const std::vector<coord_t>& DashedLineElement::getPoints() const {
        return points;
    }
    void DashedLineElement::setPoints(const std::vector<coord_t>& newPoints) {
        points = newPoints;
        validatePoints();
        setVerticesHaveChanged(true);
    }
    length_t BaseDashedLineElement::getSpaceBetweenDashes() const {
        return spaceBetweenDashes;
    }
    void BaseDashedLineElement::setSpaceBetweenDashes(length_t newSpaceBetweenDashes) {
        spaceBetweenDashes = newSpaceBetweenDashes;
        setVerticesHaveChanged(true);
    }
    length_t BaseDashedLineElement::getWidth() const {
        return width;
    }
    void BaseDashedLineElement::setWidth(length_t newWidth) {
        width = newWidth;
        setVerticesHaveChanged(true);
    }
    const color::RGB& BaseDashedLineElement::getColor() const {
        return color;
    }
    void BaseDashedLineElement::setColor(const color::RGB& newColor) {
        color = newColor;
        setVerticesHaveChanged(true);
    }
    BaseDashedLineElement::BaseDashedLineElement(length_t spaceBetweenDashes, length_t width, const color::RGB& color) :
        spaceBetweenDashes(spaceBetweenDashes), width(width), color(color) {}
    BaseDashedLineElement::~BaseDashedLineElement() = default;

    void DashedLineElement::validatePoints() {
        assert(points.size() != 1);
    }

    DashedLineElement::~DashedLineElement() = default;

}
