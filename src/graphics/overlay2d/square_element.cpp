#include "square_element.h"
#include "vertex_generator.h"
namespace bricksim::overlay2d {
    SquareElement::SquareElement(const coord_t& center, length_t sideLength, const color::RGB& color) :
        center(center), sideLength(sideLength), color(color) {}

    bool SquareElement::isPointInside(coord_t point) {
        return std::abs(point.x - center.x) < sideLength / 2.f && std::abs(point.y - center.y) < sideLength / 2.f;
    }

    unsigned int SquareElement::getVertexCount() {
        return vertex_generator::getVertexCountForSquare();
    }

    void SquareElement::writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) {
        vertex_generator::generateVerticesForSquare(buffer, center, sideLength, color, viewportSize);
    }

    const coord_t& SquareElement::getCenter() const {
        return center;
    }

    void SquareElement::setCenter(const coord_t& value) {
        SquareElement::center = value;
        setVerticesHaveChanged(true);
    }

    length_t SquareElement::getSideLength() const {
        return sideLength;
    }

    void SquareElement::setSideLength(length_t value) {
        SquareElement::sideLength = value;
        setVerticesHaveChanged(true);
    }

    const color::RGB& SquareElement::getColor() const {
        return color;
    }

    void SquareElement::setColor(const color::RGB& value) {
        SquareElement::color = value;
        setVerticesHaveChanged(true);
    }

}
