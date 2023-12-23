#pragma once
#include "../../helpers/color.h"
#include "data.h"

namespace bricksim::overlay2d::vertex_generator {
    void generateVerticesForLine(std::vector<Vertex>::iterator& buffer, coord_t start, coord_t end, length_t width, color::RGB color, coord_t viewportSize);
    constexpr unsigned int getVertexCountForLine() {
        return 6;
    }

    void generateVerticesForCCWTriangle(std::vector<Vertex>::iterator& buffer, coord_t p0, coord_t p1, coord_t p2, color::RGB color, coord_t viewportSize);
    void generateVerticesForTriangle(std::vector<Vertex>::iterator& buffer, coord_t p0, coord_t p1, coord_t p2, color::RGB color, coord_t viewportSize);
    constexpr unsigned int getVertexCountForTriangle() {
        return 3;
    }

    void generateVerticesForSquare(std::vector<Vertex>::iterator& buffer, coord_t center, length_t sideLength, color::RGB color, coord_t viewportSize);
    constexpr unsigned int getVertexCountForSquare() {
        return 6;
    }

    void generateVerticesForRegularPolygon(std::vector<Vertex>::iterator& buffer, coord_t center, length_t radius, short numEdges, color::RGB color, coord_t viewportSize);
    constexpr unsigned int getVertexCountForRegularPolygon(short numEdges) {
        return (numEdges - 2) * 3;
    }

    void generateVerticesForQuad(std::vector<Vertex>::iterator& buffer, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4, color::RGB color, coord_t viewportSize);
    constexpr unsigned int getVertexCountForQuad() {
        return 6;
    }

    void generateVerticesForPolyLine(std::vector<Vertex>::iterator& buffer, const std::vector<coord_t>& points, length_t width, color::RGB color, coord_t viewportSize);
    constexpr unsigned int getVertexCountForPolyLine(uint64_t numPoints) {
        return (numPoints - 1) * getVertexCountForQuad();
    }
    
    GLM_CONSTEXPR static glm::vec2 toNDC(coord_t coord, coord_t viewportSize) {
        return coord / viewportSize * 2.f - 1.f;
    }

    constexpr bool isNDConScreen(glm::vec2 ndc) {
        return -1.f <= ndc.x && ndc.x <= 1.f && -1.f <= ndc.y && ndc.y <= 1.f;
    }

    template<class T>
    GLM_CONSTEXPR glm::vec2 toNDC(T coord, coord_t viewportSize) = delete;//disable automatic conversion
}
