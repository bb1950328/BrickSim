#pragma once
#include "../../helpers/color.h"
#include "element.h"
namespace bricksim::overlay2d {
    class SquareElement : public Element {
    private:
        coord_t center;
        length_t sideLength;
        color::RGB color;

    public:
        SquareElement(const coord_t& center, length_t sideLength, const color::RGB& color);

        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        void writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) override;

        const coord_t& getCenter() const;
        void setCenter(const coord_t& value);
        length_t getSideLength() const;
        void setSideLength(length_t value);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& value);
    };

}
