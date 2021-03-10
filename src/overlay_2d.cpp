

#include "overlay_2d.h"

namespace overlay2d {
    Vertex::Vertex(const glm::vec2 &position, const glm::vec3 &color, element_id_t elementId) : position(position), color(color), elementId(elementId) {}

    void Collection::rebuild(element_id_t firstUnusedElementId) {
        firstElementId = firstUnusedElementId;
        lastElementIds.clear();
        for (const auto &overlay : overlays) {
            auto lastId = overlay->rebuild(firstUnusedElementId);
            lastElementIds.push_back(lastId);
            firstUnusedElementId = lastId + 1;
        }
    }

    bool Collection::clickEvent(element_id_t elementId) {
        if (elementId < firstElementId || elementId > lastElementIds.back()) {
            return false;
        }
        for (int i = 0; i < overlays.size(); ++i) {

        }
        return false;
    }

    void Graphics::addLine(glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color, element_id_t elementId) {
        // 1----------------------------------2
        // |                                  |
        // + start                        end +
        // |                                  |
        // 4----------------------------------3
        const auto startToEnd = end - start;
        const auto halfEdge = glm::normalize(glm::vec2(startToEnd.y, startToEnd.x)) * width / 2.0f;
        const glm::vec2 p1 = start - halfEdge;
        const glm::vec2 p2 = end - halfEdge;
        const glm::vec2 p3 = end + halfEdge;
        const glm::vec2 p4 = start + halfEdge;
        addQuad(color, elementId, p1, p2, p3, p4);
    }

    /**
     * p1 -- p2
     * |     |
     * p4 -- p3
     */
    void Graphics::addQuad(const util::RGBcolor &color, element_id_t elementId, const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec2 &p4) {
        vertices.emplace_back(p1, color.asGlmVector(), elementId);
        vertices.emplace_back(p4, color.asGlmVector(), elementId);
        vertices.emplace_back(p3, color.asGlmVector(), elementId);
        vertices.emplace_back(p3, color.asGlmVector(), elementId);
        vertices.emplace_back(p2, color.asGlmVector(), elementId);
        vertices.emplace_back(p1, color.asGlmVector(), elementId);
    }

    void Graphics::addTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, util::RGBcolor color, element_id_t elementId) {
        vertices.emplace_back(a, color.asGlmVector(), elementId);
        if (((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y)) > 0) {
            //already counterclockwise
            vertices.emplace_back(b, color.asGlmVector(), elementId);
            vertices.emplace_back(c, color.asGlmVector(), elementId);
        } else {
            //clockwise, we have to swap two edges
            vertices.emplace_back(c, color.asGlmVector(), elementId);
            vertices.emplace_back(b, color.asGlmVector(), elementId);
        }
    }

    void Graphics::addSquare(glm::vec2 center, float sideLength, util::RGBcolor color, element_id_t elementId) {
        const float halfSideLength = sideLength / 2;
        auto p1 = center + glm::vec2{-halfSideLength, -halfSideLength};
        auto p2 = center + glm::vec2{halfSideLength, -halfSideLength};
        auto p3 = center + glm::vec2{halfSideLength, halfSideLength};
        auto p4 = center + glm::vec2{-halfSideLength, halfSideLength};
        addQuad(color, elementId, p1, p2, p3, p4);
    }

    void Graphics::addRegularPolygon(glm::vec2 center, float radius, short numEdges, util::RGBcolor color, element_id_t elementId) {
        float angleStep = 2 * M_PI / numEdges;
        const glm::vec2 p0 = {radius + center.x, center.y};
        glm::vec2 lastP = {radius*std::cos(angleStep) + center.x, radius * std::sin(angleStep) + center.y};
        for (short i = 2; i < numEdges; ++i) {
            glm::vec2 currentP = {radius*std::cos(angleStep * i) + center.x, radius * std::sin(angleStep * i) + center.y};
            vertices.emplace_back(p0, color.asGlmVector(), elementId);
            vertices.emplace_back(lastP, color.asGlmVector(), elementId);
            vertices.emplace_back(currentP, color.asGlmVector(), elementId);
            lastP = currentP;
        }
    }
}