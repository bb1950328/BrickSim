#include "polygon_clipping.h"
#include "util.h"
#include <algorithm>
#include <cassert>

namespace bricksim::polyclip {
    Vertex::Vertex() :
        Vertex(0.f, 0.f) {}

    Vertex::Vertex(const glm::vec2& p) :
        Vertex(p.x, p.y) {}

    Vertex::Vertex(float x, float y) :
        x(x), y(y), alpha(0.0), next(nullptr), prev(nullptr), neighbour(nullptr), intersect(false), entryExit(false), processed(false) {}

    VertexPtrDistance::VertexPtrDistance(Vertex* vPtr, float dis) :
        ptr(vPtr), distance(dis) {}

    bool SortVertexPtrDistance::operator()(const VertexPtrDistance& v1, const VertexPtrDistance& v2) {
        return v1.distance < v2.distance;
    }

    PolygonVertexIterator::PolygonVertexIterator(Vertex* cv, int visitedCount) :
        currentVertex(cv), visitedCount(visitedCount) {}

    PolygonVertexIterator PolygonVertexIterator::operator++() {
        visitedCount++;
        currentVertex = currentVertex->next;
        return *this;
    }

    PolygonVertexIterator PolygonVertexIterator::operator--() {
        visitedCount--;
        currentVertex = currentVertex->prev;
        return *this;
    }

    Vertex& PolygonVertexIterator::operator*() {
        return *currentVertex;
    }

    Vertex* PolygonVertexIterator::operator->() {
        return currentVertex;
    }

    bool PolygonVertexIterator::operator!=(const PolygonVertexIterator& p) const {
        return abs(visitedCount) != abs(p.visitedCount);
    }

    Vertex* PolygonVertexIterator::eval() {
        return currentVertex;
    }

    PolygonVertexIterator PolygonVertexIterator::next() {
        PolygonVertexIterator nextIter = *this;
        return ++nextIter;
    }

    Polygon::Polygon(std::vector<glm::vec2> vertices) :
        vertexCount(0) {
        if (!vertices.empty()) {
            auto* newVertex = new Vertex(vertices[0]);
            newVertex->next = newVertex;
            newVertex->prev = newVertex;

            startVertex = newVertex;
            endVertex = newVertex;
            vertexCount++;
            for (size_t i = 1; i < vertices.size(); ++i) {
                insert(vertices[i], endVertex);
                endVertex = endVertex->next;
            }
        }
    }

    Polygon::Polygon(Polygon& poly) :
        vertexCount(0) {
        size_t i = 0;
        for (auto& iter: poly) {
            if (!iter.intersect) {
                auto* newVertex = new Vertex(iter.x, iter.y);
                if (i == 0) {
                    newVertex->next = newVertex;
                    newVertex->prev = newVertex;

                    startVertex = newVertex;
                    endVertex = newVertex;
                    vertexCount++;
                } else {
                    insert(newVertex, endVertex);
                    endVertex = endVertex->next;
                }
            }
            ++i;
        }
    }

    Polygon& Polygon::operator=(Polygon&& poly) noexcept {
        if (this == &poly) {
            return *this;
        }
        while (startVertex) {
            remove(startVertex);
        }

        auto* copy = new Polygon(poly);

        vertexCount = copy->vertexCount;
        startVertex = copy->startVertex;
        endVertex = copy->endVertex;

        return *this;
    }

    Polygon::~Polygon() {
        while (startVertex) {
            remove(startVertex);
        }
    }

    void Polygon::insert(Vertex* newVertex, Vertex* pos) {
        auto nextVertex = pos->next;

        newVertex->next = nextVertex;
        newVertex->prev = pos;

        pos->next = newVertex;
        nextVertex->prev = newVertex;

        vertexCount++;
    }

    void Polygon::insert(const glm::vec2& vertex, Vertex* pos) {
        auto* newVertex = new Vertex(vertex);
        auto* nextVertex = pos->next;

        newVertex->next = nextVertex;
        newVertex->prev = pos;

        pos->next = newVertex;
        nextVertex->prev = newVertex;

        vertexCount++;
    }

    void Polygon::insert(float x, float y, Vertex* pos) {
        auto* newVertex = new Vertex(x, y);
        auto* nextVertex = pos->next;

        newVertex->next = nextVertex;
        newVertex->prev = pos;

        pos->next = newVertex;
        nextVertex->prev = newVertex;

        vertexCount++;
    }

    void Polygon::remove(Vertex* pos) {
        if (vertexCount == 1) {
            startVertex = nullptr;
            endVertex = nullptr;
            auto neighbourVertex = pos->neighbour;
            if (neighbourVertex) {
                neighbourVertex->neighbour = nullptr;
            }
            delete pos;
        } else {
            auto previous = pos->prev;
            auto next = pos->next;
            auto neighbour = pos->neighbour;

            if (previous) {
                previous->next = next;
            }
            if (next) {
                next->prev = previous;
            }
            if (neighbour) {
                neighbour->neighbour = nullptr;
            }
            if (pos == startVertex) {
                startVertex = next;
            }
            if (pos == endVertex) {
                endVertex = previous;
            }
            delete pos;
        }
        vertexCount--;
    }

    void PolygonOperation::detectIntersection(Polygon& clipPoly, Polygon& subPoly) {
        int intersectionPointCount = 0;
        int loop1Count = 0;
        int loop1Total = clipPoly.vertexCount;

        for (auto iter1 = clipPoly.begin(); loop1Count < loop1Total; ++loop1Count) {
            auto nextCheck = iter1.next();
            int loop2Count = 0;
            int loop2Total = subPoly.vertexCount;

            std::vector<VertexPtrDistance> verticesInsertPoly1;

            for (auto iter2 = subPoly.begin(); loop2Count < loop2Total; ++loop2Count) {
                auto nextCheck2 = iter2.next();

                float alphaP = -1.0, alphaQ = -1.0;
                glm::vec2 p1(iter1->x, iter1->y);
                glm::vec2 p2(iter1->next->x, iter1->next->y);
                glm::vec2 q1(iter2->x, iter2->y);
                glm::vec2 q2(iter2->next->x, iter2->next->y);
                if (lineSegmentIntersection(p1, p2, q1, q2, alphaP, alphaQ)) {
                    intersectionPointCount++;
                    iter1->x = p1.x;
                    iter1->y = p1.y;
                    iter1->next->x = p2.x;
                    iter1->next->y = p2.y;
                    iter2->x = q1.x;
                    iter2->y = q1.y;
                    iter2->next->x = q2.x;
                    iter2->next->y = q2.y;

                    Vertex i1{alphaP * (p2 - p1) + p1};
                    Vertex i2{alphaQ * (q2 - q1) + q1};
                    auto* v1 = new Vertex(i1);
                    auto* v2 = new Vertex(i2);
                    v1->intersect = true;
                    v2->intersect = true;
                    v1->neighbour = v2;
                    v2->neighbour = v1;
                    subPoly.insert(v2, iter2.eval());

                    float distanceIntersectionPointToEdgeStartPoint = glm::length2(glm::vec2(v1->x - iter1->x, v1->y - iter1->y));
                    VertexPtrDistance vpt(v1, distanceIntersectionPointToEdgeStartPoint);
                    verticesInsertPoly1.push_back(vpt);
                }
                iter2 = nextCheck2;
            }

            // sort objects which are waiting for insert
            std::sort(verticesInsertPoly1.begin(), verticesInsertPoly1.end(), SortVertexPtrDistance());
            auto temp_cur = iter1.eval();
            for (auto item: verticesInsertPoly1) {
                clipPoly.insert(item.ptr, temp_cur);
                temp_cur = temp_cur->next;
            }
            iter1 = nextCheck;
        }
        assert(intersectionPointCount % 2 == 0 && "The intersection points should be even!");
    }

    std::pair<bool, std::vector<std::vector<glm::vec2>>> PolygonOperation::mark(Polygon& clipPoly, Polygon& subPoly, MarkType markType) {
        bool noIntersection = true;
        InsideType innerIndicator = NO_INSIDE;

        bool status;// false: exit, true: entry
        for (const auto i: {CLIP_POLY_INSIDE, SUB_POLY_INSIDE}) {
            auto& polyA = i == CLIP_POLY_INSIDE ? clipPoly : subPoly;
            auto& polyB = i == SUB_POLY_INSIDE ? clipPoly : subPoly;
            auto p0 = *(polyB.begin());
            if (pointInPolygon(glm::vec2(p0.x, p0.y), polyA)) {
                status = false;
                innerIndicator = i;
            } else {
                status = true;
            }

            for (auto& iter: polyB) {
                if (iter.intersect) {
                    iter.entryExit = status;
                    status = !status;
                    if (noIntersection) {
                        noIntersection = false;
                    }
                }
            }
        }

        std::vector<std::vector<glm::vec2>> possibleResult;

        if (noIntersection && innerIndicator == NO_INSIDE) {
            if (markType == MARK_INTERSECTION) {
                return {false, possibleResult};
            } else if (markType == MARK_UNION) {
                std::vector<glm::vec2> poly;
                for (auto& iter: clipPoly) {
                    poly.emplace_back(iter.x, iter.y);
                }
                possibleResult.push_back(poly);
                poly.clear();
                for (auto& iter: subPoly) {
                    poly.emplace_back(iter.x, iter.y);
                }
                possibleResult.push_back(poly);
                return {false, possibleResult};
            } else if (markType == MARK_DIFFERENTIATE) {
                std::vector<glm::vec2> poly;
                for (auto& iter: clipPoly) {
                    poly.emplace_back(iter.x, iter.y);
                }
                possibleResult.push_back(poly);
                return {false, possibleResult};
            }
        }
        if (noIntersection && (innerIndicator == CLIP_POLY_INSIDE || innerIndicator == SUB_POLY_INSIDE)) {
            auto& outsidePoly = innerIndicator == CLIP_POLY_INSIDE ? subPoly : clipPoly;
            auto& insidePoly = innerIndicator == CLIP_POLY_INSIDE ? clipPoly : subPoly;

            std::vector<glm::vec2> poly;
            if (markType == MARK_INTERSECTION) {
                for (auto& iter: insidePoly) {
                    poly.emplace_back(iter.x, iter.y);
                }
                possibleResult.push_back(poly);
            } else if (markType == MARK_UNION) {
                for (auto& iter: outsidePoly) {
                    poly.emplace_back(iter.x, iter.y);
                }
                possibleResult.push_back(poly);
            } else if (markType == MARK_DIFFERENTIATE) {
                for (auto& iter: outsidePoly) {
                    poly.emplace_back(iter.x, iter.y);
                }
                possibleResult.push_back(poly);
                poly.clear();
                for (auto& iter: insidePoly) {
                    poly.emplace_back(iter.x, iter.y);
                }
                possibleResult.push_back(poly);
            }
            return {false, possibleResult};
        }

        return {true, possibleResult};
    }

    std::vector<std::vector<glm::vec2>> PolygonOperation::extractIntersectionResults(Polygon& clipPoly) {
        std::vector<std::vector<glm::vec2>> results;
        for (auto ptr: getUnprocessedIntersectionPoints(clipPoly)) {
            if (ptr->processed) {
                continue;
            }

            ptr->processed = true;
            auto st = ptr;
            auto current = ptr;
            std::vector<glm::vec2> poly;
            poly.emplace_back(current->x, current->y);
            do {
                if (current->entryExit) {
                    do {
                        current = current->next;
                        poly.emplace_back(current->x, current->y);
                    } while (!current->intersect);
                } else {
                    do {
                        current = current->prev;
                        poly.emplace_back(current->x, current->y);
                    } while (!current->intersect);
                }
                current->processed = true;
                current = current->neighbour;
                current->processed = true;
            } while (current != st);
            poly.pop_back();

            results.push_back(poly);
        }

        return results;
    }

    std::vector<std::vector<glm::vec2>> PolygonOperation::extractUnionResults(Polygon& clipPoly) {
        std::vector<std::vector<glm::vec2>> results;
        for (auto ptr: getUnprocessedIntersectionPoints(clipPoly)) {
            if (ptr->processed) {
                continue;
            }

            ptr->processed = true;
            auto st = ptr;
            auto current = ptr;
            std::vector<glm::vec2> poly;
            poly.emplace_back(current->x, current->y);
            do {
                if (current->entryExit) {
                    do {
                        current = current->prev;
                        poly.emplace_back(current->x, current->y);
                    } while (!current->intersect);
                } else {
                    do {
                        current = current->next;
                        poly.emplace_back(current->x, current->y);
                    } while (!current->intersect);
                }
                current->processed = true;
                current = current->neighbour;
                current->processed = true;
            } while (current != st);
            poly.pop_back();

            results.push_back(poly);
        }

        return results;
    }
    std::vector<std::vector<glm::vec2>> PolygonOperation::extractDifferentiateResults(Polygon& clipPoly) {
        std::vector<std::vector<glm::vec2>> results;
        for (auto ptr: getUnprocessedIntersectionPoints(clipPoly)) {
            if (ptr->processed) {
                continue;
            }

            bool self = true;
            ptr->processed = true;
            auto st = ptr;
            auto current = ptr;
            std::vector<glm::vec2> poly;
            poly.emplace_back(current->x, current->y);
            do {
                if (current->entryExit) {
                    do {
                        current = self ? current->prev : current->next;
                        poly.emplace_back(current->x, current->y);
                    } while (!current->intersect);
                } else {
                    do {
                        current = self ? current->next : current->prev;
                        poly.emplace_back(current->x, current->y);
                    } while (!current->intersect);
                }
                self = !self;
                current->processed = true;
                current = current->neighbour;
                current->processed = true;
            } while (current != st);
            poly.pop_back();

            results.push_back(poly);
        }

        return results;
    }

    std::vector<Vertex*> PolygonOperation::getUnprocessedIntersectionPoints(Polygon& polygon) {
        std::vector<Vertex*> unprocessedIntersectionPoints;
        for (auto& item: polygon) {
            if (item.intersect && !item.processed) {
                unprocessedIntersectionPoints.push_back(&item);
            }
        }
        return unprocessedIntersectionPoints;
    }

    bool PolygonOperation::lineSegmentIntersection(
            glm::vec2& p1, glm::vec2& p2,
            glm::vec2& q1, glm::vec2& q2,
            float& alphaP, float& alphaQ) {
        const float perturbation = 1.001f;

        glm::vec2 vecP1Q1 = p1 - q1;
        glm::vec2 vecP2Q1 = p2 - q1;
        glm::vec2 vecQ2Q1 = q2 - q1;
        glm::vec2 vecQ2Q1rotated90(-vecQ2Q1.y, vecQ2Q1.x);
        float wecP1 = glm::dot(vecP1Q1, vecQ2Q1rotated90);
        float wecP2 = glm::dot(vecP2Q1, vecQ2Q1rotated90);

        if (wecP1 == 0.f) {
            p1 = (p1 - p2) * perturbation + p2;
            vecP1Q1 = p1 - q1;
            wecP1 = glm::dot(vecP1Q1, vecQ2Q1rotated90);
            wecP2 = glm::dot(vecP2Q1, vecQ2Q1rotated90);
        }
        if (wecP2 == 0.f) {
            p2 = (p2 - p1) * perturbation + p1;
            vecP2Q1 = p2 - q1;
            wecP1 = glm::dot(vecP1Q1, vecQ2Q1rotated90);
            wecP2 = glm::dot(vecP2Q1, vecQ2Q1rotated90);
        }
        if (wecP1 * wecP2 < 0.f) {
            glm::vec2 vecQ1P1 = q1 - p1;
            glm::vec2 vecQ2P1 = q2 - p1;
            glm::vec2 vecP2P1 = p2 - p1;
            glm::vec2 vecP2P1rotated90(-vecP2P1.y, vecP2P1.x);
            float wecQ1 = glm::dot(vecQ1P1, vecP2P1rotated90);
            float wecQ2 = glm::dot(vecQ2P1, vecP2P1rotated90);

            if (wecQ1 == 0.f) {
                q1 = (q1 - q2) * perturbation + q2;
                vecQ1P1 = q1 - p1;
                wecQ1 = glm::dot(vecQ1P1, vecP2P1rotated90);
                wecQ2 = glm::dot(vecQ2P1, vecP2P1rotated90);
            }
            if (wecQ2 == 0.f) {
                q2 = (q2 - q1) * perturbation + q1;
                vecQ2P1 = q2 - p1;
                wecQ1 = glm::dot(vecQ1P1, vecP2P1rotated90);
                wecQ2 = glm::dot(vecQ2P1, vecP2P1rotated90);
            }
            if (wecQ1 * wecQ2 <= 0.f) {
                alphaP = wecP1 / (wecP1 - wecP2);
                alphaQ = wecQ1 / (wecQ1 - wecQ2);
                return true;
            }
        }

        return false;
    }

    bool PolygonOperation::pointInPolygon(const glm::vec2& point, Polygon& polygon) {
        // See https://wrf.ecse.rpi.edu//Research/Short_Notes/pnpoly.html
        bool c = false;
        for (auto& iter: polygon) {
            c ^= (((iter.next->y > point.y) != (iter.y > point.y)) && (point.x < (iter.x - iter.next->x) * (point.y - iter.next->y) / (iter.y - iter.next->y) + iter.next->x));
        }
        return c;
    }
    void PolygonOperation::print(Polygon& polygon) {
        for (auto& iter: polygon) {
            std::cout << iter.x << " " << iter.y << "\n";
        }
    }
}