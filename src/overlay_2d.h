

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
    typedef glm::usvec2 coord_t;

    typedef short length_t;
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
        coord_t lastWrittenViewportSize{0, 0};

        unsigned int vao, vbo;

        void verticesHaveChanged(const std::shared_ptr<Element>& changedElement);
    public:
        void updateVertices(coord_t viewportSize);
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
        virtual bool isPointInside(coord_t point) = 0;
        virtual unsigned int getVertexCount() = 0;
        virtual Vertex *writeVertices(Vertex *firstVertexLocation, coord_t viewportSize) = 0;
    };

    class LineElement: public Element {
    private:
        coord_t start, end;
        float width;
        util::RGBcolor color;
    public:
        LineElement(coord_t start, coord_t end, float width, util::RGBcolor color);
        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        Vertex *writeVertices(Vertex *firstVertexLocation, coord_t viewportSize) override;
        const coord_t &getStart() const;
        void setStart(const coord_t &value);
        const coord_t &getEnd() const;
        void setEnd(const coord_t &value);
        float getWidth() const;
        void setWidth(float value);
        const util::RGBcolor &getColor() const;
        void setColor(const util::RGBcolor &value);
    };

    namespace {
        Vertex *generateVerticesForLine(Vertex *firstVertexLocation, coord_t start, coord_t end, length_t width, util::RGBcolor color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForLine();

        Vertex * generateVerticesForTriangle(Vertex *firstVertexLocation, coord_t p0, coord_t p1, coord_t p2, util::RGBcolor color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForTriangle();

        Vertex * generateVerticesForSquare(Vertex *firstVertexLocation, coord_t center, length_t sideLength, util::RGBcolor color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForSquare();

        Vertex * generateVerticesForRegularPolygon(Vertex *firstVertexLocation, coord_t center, length_t radius, short numEdges, util::RGBcolor color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForRegularPolygon(short numEdges);

        Vertex * generateVerticesForQuad(Vertex *firstVertexLocation, const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec2 &p4, util::RGBcolor color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForQuad();

        constexpr glm::vec2 toNDC(coord_t coord, coord_t viewportSize);
        constexpr glm::vec2 toNDC(glm::vec2 coord, coord_t viewportSize);
        template <class T>
        constexpr glm::vec2 toNDC(T coord, coord_t viewportSize) = delete;//disable automatic conversion
    }
}
#endif //BRICKSIM_OVERLAY_2D_H
