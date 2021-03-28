

#include "overlay_2d.h"
#include "controller.h"

#include <utility>

namespace overlay2d {
    Vertex::Vertex(const glm::vec2 &position, const glm::vec3 &color) : position(position), color(color) {}


    Element::Element(std::weak_ptr<ElementCollection> collection) : collection(std::move(collection)) {}

    void Element::verticesHaveChanged() {
        collection.lock()->verticesHaveChanged(shared_from_this());
    }

    void Element::addToCollection() {
        collection.lock()->addElement(shared_from_this());
    }

    void Element::removeFromCollection() {
        collection.lock()->removeElement(shared_from_this());
    }

    void ElementCollection::verticesHaveChanged(const std::shared_ptr<Element>& changedElement) {
        changedElements.insert(changedElement);
    }

    void ElementCollection::updateVertices() {
        bool needToRewriteEverything = false;
        std::vector<VertexRange> changedRanges;
        const auto &firstVertex = vertices.begin();
        for (const auto &elem : changedElements) {
            auto vertexRangesIt = vertexRanges.find(elem);
            const bool elementIsNew = vertexRangesIt == vertexRanges.end();
            VertexRange &range = elementIsNew ? vertexRanges[elem] = {} : vertexRangesIt->second;
            const bool elementIsDeleted = elements.find(elem) == elements.end();
            const auto newVertexCount = elem->getVertexCount();
            const bool elementVertexCountChanged = range.count != newVertexCount;

            const bool needToAppendAtEnd = elementVertexCountChanged || elementIsNew || elementIsDeleted;
            needToRewriteEverything |= needToAppendAtEnd;
            
            if (elementVertexCountChanged || elementIsDeleted) {
                vertices.erase(firstVertex + range.start, firstVertex + range.start + range.count - 1);

                //adjust ranges after current
                for (auto &item : vertexRanges) {
                    if (item.second.start > range.start) {
                        item.second.count -= range.count;
                    }
                }
            }
            if (needToAppendAtEnd) {
                const auto firstToWrite = vertices.end();
                vertices.resize(vertices.size()+newVertexCount);
                elem->writeVertices(firstToWrite);
                range.start = firstToWrite - firstVertex;
                range.count = newVertexCount;
            } else {
                elem->writeVertices(firstVertex + range.count);
                if (!needToRewriteEverything) {
                    changedRanges.push_back(range);
                }
            }
        }
        controller::executeOpenGL([&]() {
            //todo update metrics::vramUsageBytes
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            if (needToRewriteEverything) {
                glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
            } else {
                for (const auto &range : changedRanges) {
                    glBufferSubData(GL_ARRAY_BUFFER, range.start*sizeof(Vertex), range.count*sizeof(Vertex), &vertices[range.start]);//todo maybe last param is &vertices[0]
                }
            }
        });
        changedElements.clear();
    }

    ElementCollection::ElementCollection() { // NOLINT(cppcoreguidelines-pro-type-member-init)
        controller::executeOpenGL([&](){
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        });
    }

    ElementCollection::~ElementCollection() {
        controller::executeOpenGL([&](){
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
        });
    }

    void ElementCollection::draw() {
        updateVertices();
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    }

    void ElementCollection::addElement(const std::shared_ptr<Element> &element) {
        elements.insert(element);
        verticesHaveChanged(element);
    }

    void ElementCollection::removeElement(const std::shared_ptr<Element> &element) {
        elements.erase(element);
        verticesHaveChanged(element);
    }

    bool ElementCollection::hasElements() {
        return !elements.empty();
    }

    void generateVerticesForLine(std::vector<Vertex>::iterator& firstVertexLocation, glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color) {
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
        generateVerticesForQuad(firstVertexLocation, p1, p2, p3, p4, color);
    }
    constexpr unsigned int getVertexCountForLine() {
        return 6;
    }

    void generateVerticesForTriangle(std::vector<Vertex>::iterator& firstVertexLocation, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, util::RGBcolor color) {
        *firstVertexLocation = {p0, color.asGlmVector()};
        if (((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y)) > 0) {
            //already counterclockwise
            *(firstVertexLocation+1) = {p1, color.asGlmVector()};
            *(firstVertexLocation+2) = {p2, color.asGlmVector()};
        } else {
            //clockwise, we have to swap two edges
            *(firstVertexLocation+1) = {p2, color.asGlmVector()};
            *(firstVertexLocation+2) = {p1, color.asGlmVector()};
        }
        firstVertexLocation += 3;
    }
    constexpr unsigned int getVertexCountForTriangle() {
        return 3;
    }

    void generateVerticesForSquare(std::vector<Vertex>::iterator& firstVertexLocation, glm::vec2 center, float sideLength, util::RGBcolor color) {
        const float halfSideLength = sideLength / 2;
        auto p1 = center + glm::vec2{-halfSideLength, -halfSideLength};
        auto p2 = center + glm::vec2{halfSideLength, -halfSideLength};
        auto p3 = center + glm::vec2{halfSideLength, halfSideLength};
        auto p4 = center + glm::vec2{-halfSideLength, halfSideLength};
        generateVerticesForQuad(firstVertexLocation, p1, p2, p3, p4, color);
    }
    constexpr unsigned int getVertexCountForSquare() {
        return 6;
    }

    void generateVerticesForRegularPolygon(std::vector<Vertex>::iterator& firstVertexLocation, glm::vec2 center, float radius, short numEdges, util::RGBcolor color) {
        float angleStep = 2 * M_PI / numEdges;
        const glm::vec2 p0 = {radius + center.x, center.y};
        glm::vec2 lastP = {radius*std::cos(angleStep) + center.x, radius * std::sin(angleStep) + center.y};
        for (short i = 2; i < numEdges; ++i) {
            glm::vec2 currentP = {radius*std::cos(angleStep * i) + center.x, radius * std::sin(angleStep * i) + center.y};

            *firstVertexLocation = {p0, color.asGlmVector()};
            ++firstVertexLocation;

            *firstVertexLocation = {lastP, color.asGlmVector()};
            ++firstVertexLocation;

            *firstVertexLocation = {currentP, color.asGlmVector()};
            ++firstVertexLocation;

            lastP = currentP;
        }
    }
    constexpr unsigned int getVertexCountForRegularPolygon(short numEdges) {
        return numEdges-2;
    }

    void generateVerticesForQuad(std::vector<Vertex>::iterator& firstVertexLocation, const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec2 &p4, util::RGBcolor color) {
        *firstVertexLocation = {p1, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p4, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p3, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p3, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p2, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p1, color.asGlmVector()}; ++firstVertexLocation;
    }
    constexpr unsigned int getVertexCountForQuad() {
        return 6;
    }

    bool LineElement::isPointInside(glm::vec2 point) {
        return util::calculateDistanceOfPointToLine(start, end, point) <= width/2;
    }

    unsigned int LineElement::getVertexCount() {
        return getVertexCountForLine();
    }

    void LineElement::writeVertices(std::vector<Vertex>::iterator firstVertexLocation) {
        generateVerticesForLine(firstVertexLocation, start, end, width, color);
    }

    LineElement::LineElement(std::weak_ptr<ElementCollection> collection, glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color) : Element(collection), start(start), end(end), width(width), color(color) {

    }
}