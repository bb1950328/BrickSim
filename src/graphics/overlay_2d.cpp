#include "overlay_2d.h"
#include "../controller.h"
#include "../helpers/geometry.h"
#include "../helpers/util.h"
#include "glm/gtx/string_cast.hpp"
#include "spdlog/spdlog.h"
#include <cmath>
#include <glad/glad.h>
#include <palanteer.h>

namespace bricksim::overlay2d {
    namespace {
        Vertex* generateVerticesForLine(Vertex* firstVertexLocation, coord_t start, coord_t end, length_t width, color::RGB color, coord_t viewportSize) {
            // 1----------------------------------2
            // |                                  |
            // + start                        end +
            // |                                  |
            // 4----------------------------------3
            const auto startToEnd = end - start;
            const auto halfEdge = glm::normalize(glm::vec2(-startToEnd.y, startToEnd.x)) * (width / 2.f);
            const auto p1 = start - halfEdge;
            const auto p2 = start + halfEdge;
            const auto p3 = end + halfEdge;
            const auto p4 = end - halfEdge;
            return generateVerticesForQuad(firstVertexLocation, p1, p2, p3, p4, color, viewportSize);
        }

        constexpr unsigned int getVertexCountForLine() {
            return 6;
        }

        Vertex* generateVerticesForTriangle(Vertex* firstVertexLocation, coord_t p0, coord_t p1, coord_t p2, color::RGB color, coord_t viewportSize) {
            *firstVertexLocation = {toNDC(p0, viewportSize), color.asGlmVector()};
            if (((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y)) > 0) {
                //already counterclockwise
                *(firstVertexLocation + 1) = {toNDC(p1, viewportSize), color.asGlmVector()};
                *(firstVertexLocation + 2) = {toNDC(p2, viewportSize), color.asGlmVector()};
            } else {
                //clockwise, we have to swap two edges
                *(firstVertexLocation + 1) = {toNDC(p2, viewportSize), color.asGlmVector()};
                *(firstVertexLocation + 2) = {toNDC(p1, viewportSize), color.asGlmVector()};
            }
            firstVertexLocation += 3;
            return firstVertexLocation;
        }

        Vertex* generateVerticesForCCWTriangle(Vertex* firstVertexLocation, coord_t p0, coord_t p1, coord_t p2, color::RGB color, coord_t viewportSize) {
            *firstVertexLocation = {toNDC(p0, viewportSize), color.asGlmVector()};
            firstVertexLocation++;
            *firstVertexLocation = {toNDC(p1, viewportSize), color.asGlmVector()};
            firstVertexLocation++;
            *firstVertexLocation = {toNDC(p2, viewportSize), color.asGlmVector()};
            firstVertexLocation++;
            return firstVertexLocation;
        }

        constexpr unsigned int getVertexCountForTriangle() {
            return 3;
        }

        Vertex* generateVerticesForSquare(Vertex* firstVertexLocation, coord_t center, length_t sideLength, color::RGB color, coord_t viewportSize) {
            const float halfSideLength = static_cast<float>(sideLength) / 2;
            const auto x = static_cast<float>(center.x);
            const auto y = static_cast<float>(center.y);
            glm::vec2 p1(x - halfSideLength, y + halfSideLength);
            glm::vec2 p2(x + halfSideLength, y + halfSideLength);
            glm::vec2 p3(x + halfSideLength, y - halfSideLength);
            glm::vec2 p4(x - halfSideLength, y - halfSideLength);
            return generateVerticesForQuad(firstVertexLocation, p1, p2, p3, p4, color, viewportSize);
        }

        constexpr unsigned int getVertexCountForSquare() {
            return 6;
        }

        Vertex* generateVerticesForRegularPolygon(Vertex* firstVertexLocation, coord_t center, length_t radius, short numEdges, color::RGB color,
                                                  coord_t viewportSize) {
            float angleStep = 2 * static_cast<float>(M_PI) / static_cast<float>(numEdges);
            const auto cx = static_cast<float>(center.x);
            const auto cy = static_cast<float>(center.y);
            const auto fRadius = static_cast<float>(radius);
            const glm::vec2 p0 = toNDC(coord_t{fRadius + cx, cy}, viewportSize);
            glm::vec2 lastP = toNDC(glm::vec2{fRadius * std::cos(angleStep) + cx, fRadius * std::sin(angleStep) + cy}, viewportSize);
            for (short i = 2; i < numEdges; ++i) {
                const auto angle = angleStep * (i);
                glm::vec2 currentP = toNDC(glm::vec2{fRadius * std::cos(angle) + cx, fRadius * std::sin(angle) + cy}, viewportSize);

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
            return (numEdges - 2) * 3;
        }

        /**
         * p1 -- p2
         * |     |
         * p4 -- p3
         */
        Vertex* generateVerticesForQuad(Vertex* firstVertexLocation,
                                        const glm::vec2& p1,
                                        const glm::vec2& p2,
                                        const glm::vec2& p3,
                                        const glm::vec2& p4,
                                        color::RGB color,
                                        coord_t viewportSize) {
            const auto p1ndc = toNDC(p1, viewportSize);
            const auto p2ndc = toNDC(p2, viewportSize);
            const auto p3ndc = toNDC(p3, viewportSize);
            const auto p4ndc = toNDC(p4, viewportSize);
            *firstVertexLocation = {p1ndc, color.asGlmVector()};
            ++firstVertexLocation;
            *firstVertexLocation = {p4ndc, color.asGlmVector()};
            ++firstVertexLocation;
            *firstVertexLocation = {p3ndc, color.asGlmVector()};
            ++firstVertexLocation;
            *firstVertexLocation = {p3ndc, color.asGlmVector()};
            ++firstVertexLocation;
            *firstVertexLocation = {p2ndc, color.asGlmVector()};
            ++firstVertexLocation;
            *firstVertexLocation = {p1ndc, color.asGlmVector()};
            ++firstVertexLocation;
            return firstVertexLocation;
        }

        constexpr unsigned int getVertexCountForQuad() {
            return 6;
        }

        constexpr glm::vec2 toNDC(coord_t coord, coord_t viewportSize) {
            return coord / viewportSize * 2.f - 1.f;
        }

        constexpr bool isNDConScreen(glm::vec2 ndc) {
            return -1.f <= ndc.x && ndc.x <= 1.f && -1.f <= ndc.y && ndc.y <= 1.f;
        }
    }

    Vertex::Vertex(const glm::vec2& position, const glm::vec3& color) :
        position(position), color(color) {}

    Element::Element() = default;

    void Element::setVerticesHaveChanged(bool value) {
        verticesHaveChanged = value;
    }

    bool Element::haveVerticesChanged() const {
        return verticesHaveChanged;
    }
    Element::~Element() = default;
    void ElementCollection::verticesHaveChanged(const std::shared_ptr<Element>& changedElement) {
        changedElements.insert(changedElement);
    }

    void ElementCollection::updateVertices(coord_t viewportSize) {
        plScope("overlay2d::ElementCollection::updateVertices");
        const bool viewportSizeChanged = lastWrittenViewportSize != viewportSize;
        const auto& elementsToUpdate = viewportSizeChanged ? elements : changedElements;
        unsigned int changedVerticesCount = 0;
        if (hasChangedElements() || viewportSizeChanged) {
            bool needToRewriteEverything = false;
            std::vector<VertexRange> changedRanges;
            auto firstVertex = vertices.begin();
            for (const auto& elem: elementsToUpdate) {
                auto vertexRangesIt = vertexRanges.find(elem);
                const bool elementIsNew = vertexRangesIt == vertexRanges.end();
                VertexRange& range = elementIsNew ? vertexRanges[elem] : vertexRangesIt->second;
                const bool elementIsDeleted = elements.find(elem) == elements.end();
                const auto newVertexCount = elementIsDeleted ? 0 : elem->getVertexCount();
                const bool elementVertexCountChanged = range.count != newVertexCount;

                const bool needToAppendAtEnd = elementVertexCountChanged || elementIsNew || elementIsDeleted;
                needToRewriteEverything |= needToAppendAtEnd;

                if (elementVertexCountChanged) {
                    if (!elementIsNew) {
                        vertices.erase(firstVertex + range.start, firstVertex + range.start + range.count);
                    }

                    //adjust ranges after current
                    for (auto& item: vertexRanges) {
                        if (item.second.start > range.start) {
                            item.second.start -= range.count;
                        }
                    }
                }
                if (needToAppendAtEnd) {
                    range.start = static_cast<unsigned int>(vertices.size());
                    range.count = newVertexCount;
                    vertices.resize(vertices.size() + newVertexCount);
                    auto firstVertexLocation = &vertices[range.start];
                    auto lastVertexLocation = elem->writeVertices(firstVertexLocation, viewportSize);
                    assert(lastVertexLocation - firstVertexLocation == elem->getVertexCount() && "Element::writeVertices() must increment vertex pointer by vertexCount");
                } else {
                    auto firstVertexLocation = &vertices[range.start];
                    auto lastVertexLocation = elem->writeVertices(firstVertexLocation, viewportSize);
                    assert(lastVertexLocation - firstVertexLocation == elem->getVertexCount() && "Element::writeVertices() must increment vertex pointer by vertexCount");
                    if (!needToRewriteEverything) {
                        changedRanges.push_back(range);
                    }
                }
            }
            controller::executeOpenGL([this, needToRewriteEverything, &changedVerticesCount, &changedRanges]() {
                //todo update metrics::vramUsageBytes
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                if (needToRewriteEverything) {
                    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>((vertices.size() * sizeof(Vertex))), &vertices[0], GL_STATIC_DRAW);
                    changedVerticesCount = static_cast<unsigned int>(vertices.size());
                } else {
                    for (const auto& range: changedRanges) {
                        const auto offset = static_cast<GLsizeiptr>((range.start * sizeof(Vertex)));
                        const auto size = static_cast<GLsizeiptr>((range.count * sizeof(Vertex)));
                        glBufferSubData(GL_ARRAY_BUFFER, offset, size, &vertices[range.start]);
                        changedVerticesCount += range.count;
                    }
                }
            });
            changedElements.clear();
            lastWrittenViewportSize = viewportSize;
        }
    }

    ElementCollection::ElementCollection() {// NOLINT(cppcoreguidelines-pro-type-member-init)
        controller::executeOpenGL([this]() {
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
        controller::executeOpenGL([this]() {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
        });
    }

    void ElementCollection::draw() const {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    }

    void ElementCollection::addElement(const std::shared_ptr<Element>& element) {
        elements.insert(element);
        verticesHaveChanged(element);
    }

    void ElementCollection::removeElement(const std::shared_ptr<Element>& element) {
        elements.erase(element);
        verticesHaveChanged(element);
    }

    bool ElementCollection::hasElements() const {
        return !elements.empty();
    }

    bool ElementCollection::hasChangedElements() {
        //todo maybe not the best design (changing state and returning something)
        for (const auto& item: elements) {
            if (item->haveVerticesChanged()) {
                changedElements.insert(item);
                item->setVerticesHaveChanged(false);
            }
        }
        return !changedElements.empty();
    }

    bool LineElement::isPointInside(coord_t point) {
        return geometry::calculateDistanceOfPointToLine(start, end, point) <= width / 2.f;
    }

    unsigned int LineElement::getVertexCount() {
        return getVertexCountForLine();
    }

    Vertex* LineElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        return generateVerticesForLine(firstVertexLocation, start, end, width, color, viewportSize);
    }

    LineElement::LineElement(coord_t start, coord_t end, length_t width, color::RGB color) :
        start(start), end(end), width(width), color(color) {
        setVerticesHaveChanged(true);
    }

    const coord_t& LineElement::getStart() const {
        return start;
    }

    void LineElement::setStart(const coord_t& value) {
        LineElement::start = value;
        setVerticesHaveChanged(true);
    }

    const coord_t& LineElement::getEnd() const {
        return end;
    }

    void LineElement::setEnd(const coord_t& value) {
        end = value;
        setVerticesHaveChanged(true);
    }

    float LineElement::getWidth() const {
        return width;
    }

    void LineElement::setWidth(float value) {
        LineElement::width = static_cast<length_t>(value);
        setVerticesHaveChanged(true);
    }

    const color::RGB& LineElement::getColor() const {
        return color;
    }

    void LineElement::setColor(const color::RGB& value) {
        LineElement::color = value;
        setVerticesHaveChanged(true);
    }

    TriangleElement::TriangleElement(const coord_t& p0, const coord_t& p1, const coord_t& p2, const color::RGB& color) :
        p0(p0), p1(p1), p2(p2), color(color) {}

    bool TriangleElement::isPointInside(coord_t point) {
        //https://stackoverflow.com/a/2049593/8733066
        const auto d1 = (point.x - p1.x) * (p0.y - p1.y) - (p0.x - p1.x) * (point.y - p1.y);
        const auto d2 = (point.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (point.y - p2.y);
        const auto d3 = (point.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (point.y - p0.y);

        return (d1 >= 0 && d2 >= 0 && d3 >= 0) || (d1 <= 0 && d2 <= 0 && d3 <= 0);
    }

    unsigned int TriangleElement::getVertexCount() {
        return getVertexCountForTriangle();
    }

    Vertex* TriangleElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        return generateVerticesForTriangle(firstVertexLocation, p0, p1, p2, color, viewportSize);
    }

    const coord_t& TriangleElement::getP0() const {
        return p0;
    }

    void TriangleElement::setP0(const coord_t& value) {
        TriangleElement::p0 = value;
        setVerticesHaveChanged(true);
    }

    const coord_t& TriangleElement::getP1() const {
        return p1;
    }

    void TriangleElement::setP1(const coord_t& value) {
        TriangleElement::p1 = value;
        setVerticesHaveChanged(true);
    }

    const coord_t& TriangleElement::getP2() const {
        return p2;
    }

    void TriangleElement::setP2(const coord_t& value) {
        TriangleElement::p2 = value;
        setVerticesHaveChanged(true);
    }

    const color::RGB& TriangleElement::getColor() const {
        return color;
    }

    void TriangleElement::setColor(const color::RGB& value) {
        TriangleElement::color = value;
    }

    SquareElement::SquareElement(const coord_t& center, length_t sideLength, const color::RGB& color) :
        center(center), sideLength(sideLength), color(color) {}

    bool SquareElement::isPointInside(coord_t point) {
        return std::abs(point.x - center.x) < sideLength / 2.f && std::abs(point.y - center.y) < sideLength / 2.f;
    }

    unsigned int SquareElement::getVertexCount() {
        return getVertexCountForSquare();
    }

    Vertex* SquareElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        return generateVerticesForSquare(firstVertexLocation, center, sideLength, color, viewportSize);
    }

    const coord_t& SquareElement::getCenter() const {
        return center;
    }

    void SquareElement::setCenter(const coord_t& value) {
        SquareElement::center = value;
        setVerticesHaveChanged(true);
    }

    length_t SquareElement::getSideLength() const {
        return sideLength;
    }

    void SquareElement::setSideLength(length_t value) {
        SquareElement::sideLength = value;
        setVerticesHaveChanged(true);
    }

    const color::RGB& SquareElement::getColor() const {
        return color;
    }

    void SquareElement::setColor(const color::RGB& value) {
        SquareElement::color = value;
        setVerticesHaveChanged(true);
    }

    RegularPolygonElement::RegularPolygonElement(const coord_t& center, length_t radius, short numEdges, const color::RGB& color) :
        center(center), radius(radius), numEdges(numEdges), color(color) {}

    bool RegularPolygonElement::isPointInside(coord_t point) {
        //todo this is for a circle so it's wrong for points near the middle between two corners for polygons with low edge count
        // calculate the distance from center to outline for the angle between center and point
        return glm::length2(point - center) < radius * radius;
    }

    unsigned int RegularPolygonElement::getVertexCount() {
        return getVertexCountForRegularPolygon(numEdges);
    }

    Vertex* RegularPolygonElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        return generateVerticesForRegularPolygon(firstVertexLocation, center, radius, numEdges, color, viewportSize);
    }

    const coord_t& RegularPolygonElement::getCenter() const {
        return center;
    }

    void RegularPolygonElement::setCenter(const coord_t& value) {
        RegularPolygonElement::center = value;
        setVerticesHaveChanged(true);
    }

    length_t RegularPolygonElement::getRadius() const {
        return radius;
    }

    void RegularPolygonElement::setRadius(length_t value) {
        RegularPolygonElement::radius = value;
        setVerticesHaveChanged(true);
    }

    short RegularPolygonElement::getNumEdges() const {
        return numEdges;
    }

    void RegularPolygonElement::setNumEdges(short value) {
        assert(value > 2);
        RegularPolygonElement::numEdges = value;
        setVerticesHaveChanged(true);
    }

    const color::RGB& RegularPolygonElement::getColor() const {
        return color;
    }

    void RegularPolygonElement::setColor(const color::RGB& value) {
        RegularPolygonElement::color = value;
        setVerticesHaveChanged(true);
    }

    ArrowElement::ArrowElement(const coord_t& start, const coord_t& anEnd, length_t lineWidth, const color::RGB& color, float tipLengthFactor,
                               float tipWidthFactor) :
        start(start),
        end(anEnd), lineWidth(lineWidth), tipLengthFactor(tipLengthFactor),
        tipWidthFactor(tipWidthFactor), color(color) {}

    bool ArrowElement::isPointInside(coord_t point) {
        const auto normalProjection = geometry::normalProjectionOnLineClamped<2>(start, end, point);
        const auto projLengthFromEnd = normalProjection.projectionLength - normalProjection.lineLength;
        float tipWidth = calculateTipWidth();
        float tipLength = calculateTipLength();
        if (projLengthFromEnd > tipLength) {
            //on the line part
            return normalProjection.distancePointToLine < static_cast<float>(lineWidth) / 2;
        } else {
            //on the tip part
            return normalProjection.distancePointToLine < projLengthFromEnd / tipLength * tipWidth / 2;
        }
    }

    unsigned int ArrowElement::getVertexCount() {
        return 5 * getVertexCountForTriangle();
    }

    Vertex* ArrowElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        //               5
        //               |  \
        // 1-------------3      \
        // |                        \
        // start        NLE         end    (NLE=normalLineEnd)
        // |                        /
        // 2-------------4      /
        //               |  /
        //               6

        glm::vec2 startFloat = start;
        glm::vec2 endFloat = end;
        glm::vec2 fullLine = endFloat - startFloat;
        auto tipLength = calculateTipLength();
        auto fullLineLength = glm::length(fullLine);
        auto normalizedLine = glm::normalize(fullLine);
        glm::vec2 normalLineEnd = startFloat + normalizedLine * (fullLineLength - tipLength);
        const glm::vec2 halfEdge = glm::vec2(normalizedLine.y, -normalizedLine.x) * (static_cast<float>(lineWidth) / 2.0f);
        const glm::vec2 p1 = startFloat - halfEdge;
        const glm::vec2 p2 = startFloat + halfEdge;
        const glm::vec2 p3 = normalLineEnd - halfEdge;
        const glm::vec2 p4 = normalLineEnd + halfEdge;
        const glm::vec2 p5 = normalLineEnd - halfEdge * tipWidthFactor;
        const glm::vec2 p6 = normalLineEnd + halfEdge * tipWidthFactor;

        firstVertexLocation = generateVerticesForCCWTriangle(firstVertexLocation, p5, p3, end, color, viewportSize);
        firstVertexLocation = generateVerticesForCCWTriangle(firstVertexLocation, p3, p1, end, color, viewportSize);
        firstVertexLocation = generateVerticesForCCWTriangle(firstVertexLocation, p1, p2, end, color, viewportSize);
        firstVertexLocation = generateVerticesForCCWTriangle(firstVertexLocation, p2, p4, end, color, viewportSize);
        firstVertexLocation = generateVerticesForCCWTriangle(firstVertexLocation, p4, p6, end, color, viewportSize);

        return firstVertexLocation;
    }

    float ArrowElement::calculateTipLength() const {
        return static_cast<float>(lineWidth) * tipLengthFactor;
    }

    float ArrowElement::calculateTipWidth() const {
        return static_cast<float>(lineWidth) * tipWidthFactor;
    }

    const coord_t& ArrowElement::getStart() const {
        return start;
    }

    void ArrowElement::setStart(const coord_t& value) {
        ArrowElement::start = value;
        setVerticesHaveChanged(true);
    }

    const coord_t& ArrowElement::getEnd() const {
        return end;
    }

    void ArrowElement::setEnd(const coord_t& value) {
        end = value;
        setVerticesHaveChanged(true);
    }

    length_t ArrowElement::getLineWidth() const {
        return lineWidth;
    }

    void ArrowElement::setLineWidth(length_t value) {
        ArrowElement::lineWidth = value;
        setVerticesHaveChanged(true);
    }

    float ArrowElement::getTipLengthFactor() const {
        return tipLengthFactor;
    }

    void ArrowElement::setTipLengthFactor(float value) {
        ArrowElement::tipLengthFactor = value;
        setVerticesHaveChanged(true);
    }

    float ArrowElement::getTipWidthFactor() const {
        return tipWidthFactor;
    }

    void ArrowElement::setTipWidthFactor(float value) {
        ArrowElement::tipWidthFactor = value;
        setVerticesHaveChanged(true);
    }

    const color::RGB& ArrowElement::getColor() const {
        return color;
    }

    void ArrowElement::setColor(const color::RGB& value) {
        ArrowElement::color = value;
        setVerticesHaveChanged(true);
    }
    DashedLineElement::DashedLineElement(const std::vector<coord_t>& points, length_t spaceBetweenDashes, length_t width, const color::RGB& color) :
        points(points), spaceBetweenDashes(spaceBetweenDashes), width(width), color(color) {
        validatePoints();
    }
    bool DashedLineElement::isPointInside(coord_t point) {
        return false;//todo implement
    }
    unsigned int DashedLineElement::getVertexCount() {
        return getVertexCountForLine() * (points.size() - 1);
    }
    Vertex* DashedLineElement::writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) {
        Vertex* nextVertexLocation = firstVertexLocation;
        for (int i = 0; i < points.size() - 1; ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i + 1];
            const glm::vec2 delta = p2 - p1;
            const glm::vec2 halfGap(delta * (spaceBetweenDashes / glm::length(delta) / 2.f));
            if (i == 0) {
                //spdlog::debug("|delta|={}, |halfGap|={}", glm::length(delta), glm::length(halfGap));
            }
            const coord_t lineStart = p1 + halfGap;
            const coord_t lineEnd = p2 - halfGap;
            nextVertexLocation = generateVerticesForLine(nextVertexLocation, lineStart, lineEnd, width, color, viewportSize);
        }
        return nextVertexLocation;
    }
    const std::vector<coord_t>& DashedLineElement::getPoints() const {
        return points;
    }
    void DashedLineElement::setPoints(const std::vector<coord_t>& newPoints) {
        points = newPoints;
        validatePoints();
        setVerticesHaveChanged(true);
    }
    length_t DashedLineElement::getSpaceBetweenDashes() const {
        return spaceBetweenDashes;
    }
    void DashedLineElement::setSpaceBetweenDashes(length_t newSpaceBetweenDashes) {
        spaceBetweenDashes = newSpaceBetweenDashes;
        setVerticesHaveChanged(true);
    }
    length_t DashedLineElement::getWidth() const {
        return width;
    }
    void DashedLineElement::setWidth(length_t newWidth) {
        width = newWidth;
        setVerticesHaveChanged(true);
    }
    const color::RGB& DashedLineElement::getColor() const {
        return color;
    }
    void DashedLineElement::setColor(const color::RGB& newColor) {
        color = newColor;
        setVerticesHaveChanged(true);
    }
    void DashedLineElement::validatePoints() {
        assert(points.size() >= 2);
    }
}
