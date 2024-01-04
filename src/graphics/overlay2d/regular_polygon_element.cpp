#include "regular_polygon_element.h"
#include "vertex_generator.h"
#include <glm/gtx/norm.hpp>

namespace bricksim::overlay2d {
    RegularPolygonElement::RegularPolygonElement(const coord_t& center, length_t radius, short numEdges, const color::RGB& color) :
        center(center), radius(radius), numEdges(numEdges), color(color) {}

    bool RegularPolygonElement::isPointInside(coord_t point) {
        //todo this is for a circle so it's wrong for points near the middle between two corners for polygons with low edge count
        // calculate the distance from center to outline for the angle between center and point
        return glm::length2(point - center) < radius * radius;
    }

    unsigned int RegularPolygonElement::getVertexCount() {
        return vertex_generator::getVertexCountForRegularPolygon(numEdges);
    }

    void RegularPolygonElement::writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) {
        vertex_generator::generateVerticesForRegularPolygon(buffer, center, radius, numEdges, color, viewportSize);
    }

    const coord_t& RegularPolygonElement::getCenter() const {
        return center;
    }

    void RegularPolygonElement::setCenter(const coord_t& value) {
        RegularPolygonElement::center = value;
        setVerticesHaveChanged(true);
    }

    length_t RegularPolygonElement::getRadius() const {
        return radius;
    }

    void RegularPolygonElement::setRadius(length_t value) {
        RegularPolygonElement::radius = value;
        setVerticesHaveChanged(true);
    }

    short RegularPolygonElement::getNumEdges() const {
        return numEdges;
    }

    void RegularPolygonElement::setNumEdges(short value) {
        assert(value > 2);
        RegularPolygonElement::numEdges = value;
        setVerticesHaveChanged(true);
    }

    const color::RGB& RegularPolygonElement::getColor() const {
        return color;
    }

    void RegularPolygonElement::setColor(const color::RGB& value) {
        RegularPolygonElement::color = value;
        setVerticesHaveChanged(true);
    }
}
