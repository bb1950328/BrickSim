#include "line_element.h"
#include "../../helpers/geometry.h"
#include "vertex_generator.h"
namespace bricksim::overlay2d {
    bool LineElement::isPointInside(coord_t point) {
        return geometry::calculateDistanceOfPointToLine(start, end, point) <= width / 2.f;
    }

    unsigned int LineElement::getVertexCount() {
        return vertex_generator::getVertexCountForLine();
    }

    Vertex* LineElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        return vertex_generator::generateVerticesForLine(firstVertexLocation, start, end, width, color, viewportSize);
    }

    LineElement::LineElement(coord_t start, coord_t end, length_t width, color::RGB color) :
        start(start), end(end), width(width), color(color) {
        setVerticesHaveChanged(true);
    }

    const coord_t& LineElement::getStart() const {
        return start;
    }

    void LineElement::setStart(const coord_t& value) {
        LineElement::start = value;
        setVerticesHaveChanged(true);
    }

    const coord_t& LineElement::getEnd() const {
        return end;
    }

    void LineElement::setEnd(const coord_t& value) {
        end = value;
        setVerticesHaveChanged(true);
    }

    float LineElement::getWidth() const {
        return width;
    }

    void LineElement::setWidth(float value) {
        LineElement::width = static_cast<length_t>(value);
        setVerticesHaveChanged(true);
    }

    const color::RGB& LineElement::getColor() const {
        return color;
    }

    void LineElement::setColor(const color::RGB& value) {
        LineElement::color = value;
        setVerticesHaveChanged(true);
    }

}
