#include "triangle_element.h"
#include "vertex_generator.h"
namespace bricksim::overlay2d {
    TriangleElement::TriangleElement(const coord_t& p0, const coord_t& p1, const coord_t& p2, const color::RGB& color) :
        p0(p0), p1(p1), p2(p2), color(color) {}

    bool TriangleElement::isPointInside(coord_t point) {
        //https://stackoverflow.com/a/2049593/8733066
        const auto d1 = (point.x - p1.x) * (p0.y - p1.y) - (p0.x - p1.x) * (point.y - p1.y);
        const auto d2 = (point.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (point.y - p2.y);
        const auto d3 = (point.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (point.y - p0.y);

        return (d1 >= 0 && d2 >= 0 && d3 >= 0) || (d1 <= 0 && d2 <= 0 && d3 <= 0);
    }

    unsigned int TriangleElement::getVertexCount() {
        return vertex_generator::getVertexCountForTriangle();
    }

    Vertex* TriangleElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        return vertex_generator::generateVerticesForTriangle(firstVertexLocation, p0, p1, p2, color, viewportSize);
    }

    const coord_t& TriangleElement::getP0() const {
        return p0;
    }

    void TriangleElement::setP0(const coord_t& value) {
        TriangleElement::p0 = value;
        setVerticesHaveChanged(true);
    }

    const coord_t& TriangleElement::getP1() const {
        return p1;
    }

    void TriangleElement::setP1(const coord_t& value) {
        TriangleElement::p1 = value;
        setVerticesHaveChanged(true);
    }

    const coord_t& TriangleElement::getP2() const {
        return p2;
    }

    void TriangleElement::setP2(const coord_t& value) {
        TriangleElement::p2 = value;
        setVerticesHaveChanged(true);
    }

    const color::RGB& TriangleElement::getColor() const {
        return color;
    }

    void TriangleElement::setColor(const color::RGB& value) {
        TriangleElement::color = value;
    }
}
