#include "arrow_element.h"
#include "../../helpers/geometry.h"
#include "vertex_generator.h"

namespace bricksim::overlay2d {
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
        return 5 * vertex_generator::getVertexCountForTriangle();
    }

    void ArrowElement::writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) {
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

        vertex_generator::generateVerticesForCCWTriangle(buffer, p5, p3, end, color, viewportSize);
        vertex_generator::generateVerticesForCCWTriangle(buffer, p3, p1, end, color, viewportSize);
        vertex_generator::generateVerticesForCCWTriangle(buffer, p1, p2, end, color, viewportSize);
        vertex_generator::generateVerticesForCCWTriangle(buffer, p2, p4, end, color, viewportSize);
        vertex_generator::generateVerticesForCCWTriangle(buffer, p4, p6, end, color, viewportSize);
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
}
