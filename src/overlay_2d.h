

#ifndef BRICKSIM_OVERLAY_2D_H
#define BRICKSIM_OVERLAY_2D_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "types.h"
#include "helpers/util.h"

namespace overlay2d {
    class Vertex {
        glm::vec2 position;
        glm::vec3 color;
        element_id_t elementId;
    public:
        Vertex(const glm::vec2 &position, const glm::vec3 &color, element_id_t elementId);
    };

    class Graphics {
        std::vector<Vertex> vertices;
    public:
        void addLine(glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color, element_id_t elementId);
        void addTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, util::RGBcolor color, element_id_t elementId);
        void addSquare(glm::vec2 center, float sideLength, util::RGBcolor color, element_id_t elementId);
        void addRegularPolygon(glm::vec2 center, float radius, short numEdges, util::RGBcolor color, element_id_t elementId);
        void addQuad(const util::RGBcolor &color, element_id_t elementId, const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec2 &p4);
    };

    class Overlay2d {
    public:
        virtual element_id_t rebuild(element_id_t firstElementId) = 0;//return is last used element id
        virtual void click(element_id_t elementId) = 0;
    };

    class Collection {
    private:
        std::vector<std::shared_ptr<Overlay2d>> overlays;
        element_id_t firstElementId;
        std::vector<element_id_t> lastElementIds;
    public:
        void rebuild(element_id_t firstUnusedElementId);
        bool clickEvent(element_id_t elementId);
    };
}
#endif //BRICKSIM_OVERLAY_2D_H
