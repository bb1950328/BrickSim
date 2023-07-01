#pragma once
#include "../../helpers/color.h"
#include "element.h"
namespace bricksim::overlay2d {
    class BaseDashedLineElement : public Element {
    protected:
        length_t spaceBetweenDashes;
        length_t width;
        color::RGB color;
        BaseDashedLineElement(length_t spaceBetweenDashes, length_t width, const color::RGB& color);

    public:
        length_t getSpaceBetweenDashes() const;
        void setSpaceBetweenDashes(length_t newSpaceBetweenDashes);
        length_t getWidth() const;
        void setWidth(length_t newWidth);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& newColor);
        ~BaseDashedLineElement() override;
    };
    class DashedLineElement : public BaseDashedLineElement {
    private:
        std::vector<coord_t> points;
        void validatePoints();

    public:
        DashedLineElement(const std::vector<coord_t>& points, length_t spaceBetweenDashes, length_t width, const color::RGB& color);
        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) override;
        const std::vector<coord_t>& getPoints() const;
        void setPoints(const std::vector<coord_t>& newPoints);
        ~DashedLineElement() override;
    };

}
