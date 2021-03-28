

#include "overlay_2d.h"
#include "controller.h"

#include <utility>

namespace overlay2d {
    Vertex::Vertex(const glm::vec2 &position, const glm::vec3 &color) : position(position), color(color) {}


    Element::Element()= default;

    void Element::setVerticesHaveChanged(bool value) {
        verticesHaveChanged = value;
    }

    bool Element::haveVerticesChanged() const {
        return verticesHaveChanged;
    }

    void ElementCollection::verticesHaveChanged(const std::shared_ptr<Element>& changedElement) {
        changedElements.insert(changedElement);
    }

    void ElementCollection::updateVertices() {
        if (hasChangedElements()) {
            bool needToRewriteEverything = false;
            std::vector<VertexRange> changedRanges;
            auto firstVertex = vertices.begin();
            for (const auto &elem : changedElements) {
                auto vertexRangesIt = vertexRanges.find(elem);
                const bool elementIsNew = vertexRangesIt == vertexRanges.end();
                VertexRange &range = elementIsNew ? vertexRanges[elem] = {} : vertexRangesIt->second;
                const bool elementIsDeleted = elements.find(elem) == elements.end();
                const auto newVertexCount = elementIsDeleted ? 0 : elem->getVertexCount();
                const bool elementVertexCountChanged = range.count != newVertexCount;

                const bool needToAppendAtEnd = elementVertexCountChanged || elementIsNew || elementIsDeleted;
                needToRewriteEverything |= needToAppendAtEnd;

                if (elementVertexCountChanged) {
                    if (!elementIsNew) {
                        vertices.erase(firstVertex + range.start, firstVertex + range.start + range.count - 1);
                    }

                    //adjust ranges after current
                    for (auto &item : vertexRanges) {
                        if (item.second.start > range.start) {
                            item.second.count -= range.count;
                        }
                    }
                }
                if (needToAppendAtEnd) {
                    range.start = vertices.size();
                    range.count = newVertexCount;
                    vertices.resize(vertices.size() + newVertexCount);
                    elem->writeVertices(&vertices[range.start]);
                } else {
                    elem->writeVertices(&vertices[range.start]);
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
                    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
                } else {
                    for (const auto &range : changedRanges) {
                        glBufferSubData(GL_ARRAY_BUFFER, range.start * sizeof(Vertex), range.count * sizeof(Vertex), &vertices[range.start]);//todo maybe last param is &vertices[0]
                    }
                }
            });
            changedElements.clear();
        }
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

    bool ElementCollection::hasChangedElements() {
        //todo maybe not the best design (changing state and returning something)
        for (auto &item : elements) {
            if (item->haveVerticesChanged()) {
                changedElements.insert(item);
                item->setVerticesHaveChanged(false);
            }
        }
        return !changedElements.empty();
    }

    Vertex * generateVerticesForLine(Vertex *firstVertexLocation, glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color) {
        // 1----------------------------------2
        // |                                  |
        // + start                        end +
        // |                                  |
        // 4----------------------------------3
        const auto startToEnd = end - start;
        const auto halfEdge = glm::normalize(glm::vec2(-startToEnd.x, startToEnd.y)) * width / 2.0f;
        const glm::vec2 p1 = start - halfEdge;
        const glm::vec2 p2 = start + halfEdge;
        const glm::vec2 p3 = end + halfEdge;
        const glm::vec2 p4 = end - halfEdge;
        std::cout << "line(start=" << start.x << "/" << start.y << ", end=" << end.x << "/" << end.y << ")" << std::endl;
        std::cout << "p1=" << p1.x << ", " << p1.y << std::endl;
        std::cout << "p2=" << p2.x << ", " << p2.y << std::endl;
        std::cout << "p3=" << p3.x << ", " << p3.y << std::endl;
        std::cout << "p4=" << p4.x << ", " << p4.y << std::endl;
        std::cout << "startToEnd=" << startToEnd.x << ", " << startToEnd.y << std::endl;
        std::cout << "halfEdge=" << halfEdge.x << ", " << halfEdge.y << std::endl;
        return generateVerticesForQuad(firstVertexLocation, p1, p2, p3, p4, color);
    }
    constexpr unsigned int getVertexCountForLine() {
        return 6;
    }

    Vertex * generateVerticesForTriangle(Vertex *firstVertexLocation, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, util::RGBcolor color) {
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
        return firstVertexLocation;
    }
    constexpr unsigned int getVertexCountForTriangle() {
        return 3;
    }

    Vertex * generateVerticesForSquare(Vertex *firstVertexLocation, glm::vec2 center, float sideLength, util::RGBcolor color) {
        const float halfSideLength = sideLength / 2;
        auto p1 = center + glm::vec2{-halfSideLength, -halfSideLength};
        auto p2 = center + glm::vec2{halfSideLength, -halfSideLength};
        auto p3 = center + glm::vec2{halfSideLength, halfSideLength};
        auto p4 = center + glm::vec2{-halfSideLength, halfSideLength};
        return generateVerticesForQuad(firstVertexLocation, p1, p2, p3, p4, color);
    }
    constexpr unsigned int getVertexCountForSquare() {
        return 6;
    }

    Vertex * generateVerticesForRegularPolygon(Vertex *firstVertexLocation, glm::vec2 center, float radius, short numEdges, util::RGBcolor color) {
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
        return firstVertexLocation;
    }
    constexpr unsigned int getVertexCountForRegularPolygon(short numEdges) {
        return numEdges-2;
    }

    Vertex * generateVerticesForQuad(Vertex *firstVertexLocation, const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const glm::vec2 &p4, util::RGBcolor color) {
        *firstVertexLocation = {p1, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p4, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p3, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p3, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p2, color.asGlmVector()}; ++firstVertexLocation;
        *firstVertexLocation = {p1, color.asGlmVector()}; ++firstVertexLocation;
        return firstVertexLocation;
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

    Vertex * LineElement::writeVertices(Vertex *firstVertexLocation) {
        return generateVerticesForLine(firstVertexLocation, start, end, width, color);
    }

    LineElement::LineElement(glm::vec2 start, glm::vec2 end, float width, util::RGBcolor color) : start(start), end(end), width(width), color(color) {
        setVerticesHaveChanged(true);
    }

    const glm::vec2 &LineElement::getStart() const {
        return start;
    }

    void LineElement::setStart(const glm::vec2 &value) {
        LineElement::start = value;
        setVerticesHaveChanged(true);
    }

    const glm::vec2 &LineElement::getEnd() const {
        return end;
    }

    void LineElement::setEnd(const glm::vec2 &value) {
        end = value;
        setVerticesHaveChanged(true);
    }

    float LineElement::getWidth() const {
        return width;
    }

    void LineElement::setWidth(float value) {
        LineElement::width = value;
        setVerticesHaveChanged(true);
    }

    const util::RGBcolor &LineElement::getColor() const {
        return color;
    }

    void LineElement::setColor(const util::RGBcolor &value) {
        LineElement::color = value;
        setVerticesHaveChanged(true);
    }
}