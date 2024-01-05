#pragma once
#include "../../helpers/color.h"
#include "element.h"

namespace bricksim::overlay2d {
    class TriangleElement : public Element {
    private:
        coord_t p0, p1, p2;
        color::RGB color;

    public:
        TriangleElement(const coord_t& p0, const coord_t& p1, const coord_t& p2, const color::RGB& color);

        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        void writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) override;

        const coord_t& getP0() const;
        void setP0(const coord_t& value);
        const coord_t& getP1() const;
        void setP1(const coord_t& value);
        const coord_t& getP2() const;
        void setP2(const coord_t& value);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& value);
    };
}
