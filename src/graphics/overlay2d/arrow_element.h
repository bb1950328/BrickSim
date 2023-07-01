#pragma once
#include "../../helpers/color.h"
#include "element.h"
namespace bricksim::overlay2d {
    /*                        ▨       🡡
     *                        ▨▨      |
     *           🡡 ▨▨▨▨▨▨▨▨▨▨▨    |
     *           | ▨▨▨▨▨▨▨▨▨▨▨▨▨  |
     * lineWidth | ▨▨▨▨▨▨▨▨▨▨▨▨▨  | lineWidth*tipWidthFactor
     *           🡣 ▨▨▨▨▨▨▨▨▨▨▨    |
     *                         ▨▨     |
     *                         ▨      🡣
     *                         🡠--🡢
     *                         lineWidth*tipLengthFactor
     *
     * tip is at `end` point.
     */
    class ArrowElement : public Element {
    private:
        coord_t start, end;
        length_t lineWidth;
        float tipLengthFactor, tipWidthFactor;
        color::RGB color;

        float calculateTipLength() const;
        float calculateTipWidth() const;

    public:
        ArrowElement(const coord_t& start, const coord_t& anEnd, length_t lineWidth, const color::RGB& color,
                     float tipLengthFactor = 1.5f, float tipWidthFactor = 2.0f);

        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) override;

        const coord_t& getStart() const;
        void setStart(const coord_t& value);
        const coord_t& getEnd() const;
        void setEnd(const coord_t& value);
        length_t getLineWidth() const;
        void setLineWidth(length_t value);
        float getTipLengthFactor() const;
        void setTipLengthFactor(float value);
        float getTipWidthFactor() const;
        void setTipWidthFactor(float value);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& value);
    };

}
