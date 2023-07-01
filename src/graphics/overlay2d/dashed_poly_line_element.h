#pragma once
#include "dashed_line_element.h"
#include <optional>
namespace bricksim::overlay2d {
    class DashedPolyLineElement : public BaseDashedLineElement {
    public:
        using points_t = std::vector<std::vector<coord_t>>;

        DashedPolyLineElement(length_t spaceBetweenDashes, length_t width, const color::RGB& color);
        void setPoints(const points_t& origPoints);
        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        void writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) override;
        ~DashedPolyLineElement() override;

    private:
        points_t points;
        std::pair<size_t, std::optional<glm::vec2>> cutStartEnd(const std::vector<glm::vec2>& origLine, bool start);
    };

}
