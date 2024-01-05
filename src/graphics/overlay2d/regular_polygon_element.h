#pragma once
#include "../../helpers/color.h"
#include "element.h"

namespace bricksim::overlay2d {
    class RegularPolygonElement : public Element {
    private:
        coord_t center;
        length_t radius;
        short numEdges;
        color::RGB color;

    public:
        RegularPolygonElement(const coord_t& center, length_t radius, short numEdges, const color::RGB& color);

        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        void writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) override;

        const coord_t& getCenter() const;
        void setCenter(const coord_t& value);
        length_t getRadius() const;
        void setRadius(length_t value);
        short getNumEdges() const;
        void setNumEdges(short value);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& value);
    };
}
