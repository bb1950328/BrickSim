

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
        std::set<std::shared_ptr<Element>> elements;
        std::set<std::shared_ptr<Element>> changedElements;
        std::map<std::shared_ptr<Element>, VertexRange> vertexRanges;
        std::vector<Vertex> vertices;

        unsigned int vao, vbo;

        void verticesHaveChanged(const std::shared_ptr<Element>& changedElement);
    public:
        void updateVertices();
        void draw();

        void addElement(const std::shared_ptr<Element>& element);
        void removeElement(const std::shared_ptr<Element>& element);

        ElementCollection();
        ElementCollection & operator=(ElementCollection&) = delete;
        ElementCollection(const ElementCollection&) = delete;
        virtual ~ElementCollection();
        bool hasElements();
        bool hasChangedElements();
    };

    class Element : std::enable_shared_from_this<Element> {
    private:
        bool verticesHaveChanged = false;
    public:
        explicit Element();
        bool haveVerticesChanged() const;
        void setVerticesHaveChanged(bool value);
        virtual bool isPointInside(glm::vec2 point) = 0;
        virtual unsigned int getVertexCount() = 0;
        virtual Vertex * writeVertices(Vertex *firstVertexLocation) = 0;
    };

    class LineElement: public Element {
    private:
        glm::vec2 start, end;
        float width;
        util::RGBcolor color;
    public:
        LineElement(glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color);
        bool isPointInside(glm::vec2 point) override;
        unsigned int getVertexCount() override;
        Vertex * writeVertices(Vertex *firstVertexLocation) override;
        const glm::vec2 &getStart() const;
        void setStart(const glm::vec2 &value);
        const glm::vec2 &getEnd() const;
        void setEnd(const glm::vec2 &value);
        float getWidth() const;
        void setWidth(float value);
        const util::RGBcolor &getColor() const;
        void setColor(const util::RGBcolor &value);
    };

    Vertex * generateVerticesForLine(Vertex *firstVertexLocation, glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color);
    constexpr unsigned int getVertexCountForLine();

    Vertex * generateVerticesForTriangle(Vertex *firstVertexLocation, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, util::RGBcolor color);
    constexpr unsigned int getVertexCountForTriangle();

    Vertex * generateVerticesForSquare(Vertex *firstVertexLocation, glm::vec2 center, float sideLength, util::RGBcolor color);
    constexpr unsigned int getVertexCountForSquare();

    Vertex * generateVerticesForRegularPolygon(Vertex *firstVertexLocation, glm::vec2 center, float radius, short numEdges, util::RGBcolor color);
    constexpr unsigned int getVertexCountForRegularPolygon(short numEdges);

    /**
     * p1 -- p2
     * |     |
     * p4 -- p3
     */
    Vertex * generateVerticesForQuad(Vertex *firstVertexLocation, const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec2 &p4, util::RGBcolor color);
    constexpr unsigned int getVertexCountForQuad();
}
#endif //BRICKSIM_OVERLAY_2D_H
