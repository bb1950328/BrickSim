

#ifndef BRICKSIM_OVERLAY_2D_H
#define BRICKSIM_OVERLAY_2D_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include "types.h"
#include "helpers/util.h"

namespace overlay2d {
    class Vertex {
    public:
        glm::vec2 position;
        glm::vec3 color;
        Vertex(const glm::vec2 &position, const glm::vec3 &color);
        Vertex() = default;
    };

    struct VertexRange {
        unsigned int start;
        unsigned int count;
    };

    class Element;

    class ElementCollection {
    private:
        std::vector<std::shared_ptr<Element>> elements;
        std::set<std::shared_ptr<Element>> changedElements;
        std::map<std::shared_ptr<Element>, VertexRange> vertexRanges;
        std::vector<Vertex> vertices;

        unsigned int vao, vbo;

        void updateVertices();
    public:
        void verticesHaveChanged(const std::shared_ptr<Element>& changedElement);

        ElementCollection();
        ElementCollection & operator=(ElementCollection&) = delete;
        ElementCollection(const ElementCollection&) = delete;
        virtual ~ElementCollection();
    };

    class Element : std::enable_shared_from_this<Element> {
    private:
        const std::weak_ptr<ElementCollection> collection;
    protected:
        void verticesHaveChanged();
    public:
        explicit Element(std::weak_ptr<ElementCollection> collection);
        virtual bool isPointInside(glm::usvec2 point) = 0;
        virtual unsigned int getVertexCount() = 0;
        virtual void writeVertices(std::vector<Vertex>::iterator firstVertexLocation) = 0;
    };

    void generateVerticesForLine(std::vector<Vertex>::iterator& firstVertexLocation, glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color);
    constexpr unsigned int getVertexCountForLine();

    void generateVerticesForTriangle(std::vector<Vertex>::iterator& firstVertexLocation, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, util::RGBcolor color);
    constexpr unsigned int getVertexCountForTriangle();

    void generateVerticesForSquare(std::vector<Vertex>::iterator& firstVertexLocation, glm::vec2 center, float sideLength, util::RGBcolor color);
    constexpr unsigned int getVertexCountForSquare();

    void generateVerticesForRegularPolygon(std::vector<Vertex>::iterator& firstVertexLocation, glm::vec2 center, float radius, short numEdges, util::RGBcolor color);
    constexpr unsigned int getVertexCountForRegularPolygon(short numEdges);

    /**
     * p1 -- p2
     * |     |
     * p4 -- p3
     */
    void generateVerticesForQuad(std::vector<Vertex>::iterator& firstVertexLocation, const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec2 &p4, util::RGBcolor color);
    constexpr unsigned int getVertexCountForQuad();
}
#endif //BRICKSIM_OVERLAY_2D_H
