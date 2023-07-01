#pragma once

#include "../helpers/color.h"
#include "../types.h"
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <vector>
//todo split this into multiple files
namespace bricksim::overlay2d {
    typedef glm::vec2 coord_t;

    typedef float length_t;

    class Vertex {
    public:
        glm::vec2 position;
        glm::vec3 color;
        Vertex(const glm::vec2& position, const glm::vec3& color);
        Vertex() = default;
    };

    struct VertexRange {
        unsigned int start;
        unsigned int count;
    };

    class Element;

    class ElementCollection {
    private:
        uoset_t<std::shared_ptr<Element>> elements;
        uoset_t<std::shared_ptr<Element>> changedElements;
        uomap_t<std::shared_ptr<Element>, VertexRange> vertexRanges;
        std::vector<Vertex> vertices;
        coord_t lastWrittenViewportSize{0, 0};

        unsigned int vao;
        unsigned int vbo;

        void verticesHaveChanged(const std::shared_ptr<Element>& changedElement);

    public:
        void updateVertices(coord_t viewportSize);
        void draw() const;

        void addElement(const std::shared_ptr<Element>& element);
        void removeElement(const std::shared_ptr<Element>& element);

        ElementCollection();
        ElementCollection& operator=(ElementCollection&) = delete;
        ElementCollection(const ElementCollection&) = delete;
        virtual ~ElementCollection();
        [[nodiscard]] bool hasElements() const;
        bool hasChangedElements();
    };

    class Element : private std::enable_shared_from_this<Element> {
    private:
        bool verticesHaveChanged = false;

    public:
        explicit Element();
        bool haveVerticesChanged() const;
        void setVerticesHaveChanged(bool value);
        virtual bool isPointInside(coord_t point) = 0;
        virtual unsigned int getVertexCount() = 0;
        virtual Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) = 0;//todo change firstVertexLocation to non-const reference to an iterator and return type to void
        virtual ~Element();
    };

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

    class DashedPolyLineElement : public BaseDashedLineElement {
    public:
        using points_t = std::vector<std::vector<coord_t>>;

    private:
        points_t points;

    public:
        DashedPolyLineElement(length_t spaceBetweenDashes, length_t width, const color::RGB& color);
        void setPoints(const points_t& origPoints);
        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) override;
        ~DashedPolyLineElement() override;
        std::pair<size_t, std::optional<glm::vec2>> cutStartEnd(const std::vector<glm::vec2>& origLine, bool start);
    };

    class TriangleElement : public Element {
    private:
        coord_t p0, p1, p2;
        color::RGB color;

    public:
        TriangleElement(const coord_t& p0, const coord_t& p1, const coord_t& p2, const color::RGB& color);

        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) override;

        const coord_t& getP0() const;
        void setP0(const coord_t& value);
        const coord_t& getP1() const;
        void setP1(const coord_t& value);
        const coord_t& getP2() const;
        void setP2(const coord_t& value);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& value);
    };

    class SquareElement : public Element {
    private:
        coord_t center;
        length_t sideLength;
        color::RGB color;

    public:
        SquareElement(const coord_t& center, length_t sideLength, const color::RGB& color);

        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) override;

        const coord_t& getCenter() const;
        void setCenter(const coord_t& value);
        length_t getSideLength() const;
        void setSideLength(length_t value);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& value);
    };

    class RegularPolygonElement : public Element {
    private:
        coord_t center;
        length_t radius;
        short numEdges;
        color::RGB color;

    public:
        RegularPolygonElement(const coord_t& center, length_t radius, short numEdges, const color::RGB& color);

        bool isPointInside(coord_t point) override;
        unsigned int getVertexCount() override;
        Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) override;

        const coord_t& getCenter() const;
        void setCenter(const coord_t& value);
        length_t getRadius() const;
        void setRadius(length_t value);
        short getNumEdges() const;
        void setNumEdges(short value);
        const color::RGB& getColor() const;
        void setColor(const color::RGB& value);
    };

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

    namespace {
        Vertex* generateVerticesForLine(Vertex* firstVertexLocation, coord_t start, coord_t end, length_t width, color::RGB color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForLine();

        Vertex* generateVerticesForCCWTriangle(Vertex* firstVertexLocation, coord_t p0, coord_t p1, coord_t p2, color::RGB color, coord_t viewportSize);
        Vertex* generateVerticesForTriangle(Vertex* firstVertexLocation, coord_t p0, coord_t p1, coord_t p2, color::RGB color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForTriangle();

        Vertex* generateVerticesForSquare(Vertex* firstVertexLocation, coord_t center, length_t sideLength, color::RGB color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForSquare();

        Vertex* generateVerticesForRegularPolygon(Vertex* firstVertexLocation, coord_t center, length_t radius, short numEdges, color::RGB color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForRegularPolygon(short numEdges);

        Vertex* generateVerticesForQuad(Vertex* firstVertexLocation, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4, color::RGB color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForQuad();

        Vertex* generateVerticesForPolyLine(Vertex* firstVertexLocation, const std::vector<coord_t>& points, length_t width, color::RGB color, coord_t viewportSize);
        constexpr unsigned int getVertexCountForPolyLine(uint64_t numPoints);

        constexpr glm::vec2 toNDC(coord_t coord, coord_t viewportSize);
        constexpr bool isNDConScreen(glm::vec2 ndc);

        template<class T>
        constexpr glm::vec2 toNDC(T coord, coord_t viewportSize) = delete;//disable automatic conversion
    }
}
