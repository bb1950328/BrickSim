#pragma once
#include "../../helpers/color.h"
#include "element.h"
namespace bricksim::overlay2d {
    class LineElement : public Element {
    private:
        coord_t start, end;
        length_t width;
        color::RGB color;

    public:
        LineElement(coord_t start, coord_t end, length_t width, color::RGB color);
        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) override;

        const coord_t& getStart() const;
        void setStart(const coord_t& value);
        const coord_t& getEnd() const;
        void setEnd(const coord_t& value);
        float getWidth() const;
        void setWidth(float value);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& value);
    };
}
