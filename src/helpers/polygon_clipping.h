#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace bricksim::polyclip {

    struct Vertex {
        float x, y;
        Vertex* next;
        Vertex* prev;
        bool intersect;
        bool entryExit;
        Vertex* neighbour;
        float alpha;
        bool processed;

        Vertex();
        explicit Vertex(const glm::vec2& p);
        Vertex(float x, float y);
    };

    struct VertexPtrDistance {
        Vertex* ptr;
        float distance;
        VertexPtrDistance(Vertex* vPtr, float dis);
    };

    struct SortVertexPtrDistance {
        bool operator()(const VertexPtrDistance& v1, const VertexPtrDistance& v2);
    };

    class PolygonVertexIterator {
    public:
        PolygonVertexIterator(Vertex* cv, int visitedCount);
        ;

        PolygonVertexIterator operator++();
        PolygonVertexIterator operator--();
        Vertex& operator*();

        Vertex* operator->();

        bool operator!=(const PolygonVertexIterator& p) const;

        Vertex* eval();

        PolygonVertexIterator next();

    private:
        Vertex* currentVertex;
        int visitedCount;
    };

    class utils {
    };

    class Polygon {
        Vertex* startVertex;
        Vertex* endVertex;

    public:
        typedef PolygonVertexIterator iterator;
        int vertexCount;

        explicit Polygon(std::vector<glm::vec2> vertices);
        ~Polygon();

        Polygon(Polygon& poly);
        Polygon& operator=(Polygon&& poly) noexcept;

        void insert(Vertex* newVertex, Vertex* pos);
        void insert(const glm::vec2& vertex, Vertex* pos);
        void insert(float x, float y, Vertex* pos);
        void remove(Vertex* pos);

        iterator begin() {
            return {startVertex, 0};
        }

        iterator end() {
            return {endVertex, vertexCount};
        }

        iterator endEdge() {
            return {endVertex, vertexCount + 1};
        }

    protected:
        friend class PolygonVertexIterator;
    };

    enum MarkType {
        MARK_INTERSECTION,
        MARK_UNION,
        MARK_DIFFERENTIATE
    };

    enum InsideType {
        NO_INSIDE,
        CLIP_POLY_INSIDE,
        SUB_POLY_INSIDE,
    };

    /**
     * Polygons should be in CW order
     */
    class PolygonOperation {
    public:
        static void detectIntersection(Polygon& clipPoly, Polygon& subPoly);

        /**
         * @return true=normal intersection, false=no intersection or one polygon is inside the other one
         */
        static std::pair<bool, std::vector<std::vector<glm::vec2>>> mark(Polygon& clipPoly, Polygon& subPoly, MarkType markType);

        static std::vector<std::vector<glm::vec2>> extractIntersectionResults(Polygon& clipPoly);
        static std::vector<std::vector<glm::vec2>> extractUnionResults(Polygon& clipPoly);
        static std::vector<std::vector<glm::vec2>> extractDifferentiateResults(Polygon& clipPoly);

        static void print(Polygon& polygon);

    private:
        static bool lineSegmentIntersection(glm::vec2& p1, glm::vec2& p2,
                                            glm::vec2& q1, glm::vec2& q2,
                                            float& alphaP, float& alphaQ);

        static bool pointInPolygon(const glm::vec2& point, Polygon& polygon);
        static std::vector<Vertex*> getUnprocessedIntersectionPoints(Polygon& polygon);
    };
}
