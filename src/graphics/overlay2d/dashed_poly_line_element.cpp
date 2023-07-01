#include "dashed_poly_line_element.h"
#include "vertex_generator.h"
#include <numeric>
namespace bricksim::overlay2d {
    DashedPolyLineElement::DashedPolyLineElement(length_t spaceBetweenDashes, length_t width, const color::RGB& color) :
        BaseDashedLineElement(spaceBetweenDashes, width, color) {}

    bool DashedPolyLineElement::isPointInside(coord_t point) {
        return false;
    }
    unsigned int DashedPolyLineElement::getVertexCount() {
        return std::transform_reduce(points.cbegin(),
                                     points.cend(),
                                     0,
                                     std::plus{},
                                     [](auto line) {
                                         return vertex_generator::getVertexCountForPolyLine(line.size());
                                     });
    }
    void DashedPolyLineElement::writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) {
        for (const auto& item: points) {
            vertex_generator::generateVerticesForPolyLine(buffer, item, width, color, viewportSize);
        }
    }
    DashedPolyLineElement::~DashedPolyLineElement() = default;

    std::pair<size_t, std::optional<glm::vec2>> DashedPolyLineElement::cutStartEnd(const std::vector<glm::vec2>& origLine, bool start) {
        auto remaining = spaceBetweenDashes / 2;
        auto iStep = start ? 1 : -1;
        size_t i = start ? 0 : (origLine.size() - 1);
        std::optional<glm::vec2> additionalPoint = std::nullopt;
        while (remaining > 0) {
            const auto segment = origLine[i + iStep] - origLine[i];
            const auto segmentLength = glm::length(segment);
            if (segmentLength < remaining) {
                additionalPoint = origLine[i] + segment / segmentLength * remaining;
            }
            i += iStep;
            remaining -= segmentLength;
        }
        return {i, additionalPoint};
    }

    void DashedPolyLineElement::setPoints(const points_t& origPoints) {
        this->points.clear();
        this->points.reserve(origPoints.size());
        for (size_t i = 0; i < origPoints.size(); ++i) {
            const auto& origLine = origPoints[i];
            const auto [iFirstToCopy, firstPoint] = i > 0
                                                            ? cutStartEnd(origLine, true)
                                                            : std::make_pair(0, std::nullopt);
            const auto [iLastToCopy, lastPoint] = i < origPoints.size() - 1
                                                          ? cutStartEnd(origLine, false)
                                                          : std::make_pair(origLine.size() - 1, std::nullopt);

            std::vector<coord_t> newLine;
            newLine.reserve(origLine.size());

            if (firstPoint.has_value()) {
                newLine.push_back(*firstPoint);
            }
            for (size_t j = iFirstToCopy; j <= iLastToCopy; ++j) {
                newLine.push_back(origLine[j]);
            }
            if (lastPoint.has_value()) {
                newLine.push_back(*lastPoint);
            }

            this->points.push_back(newLine);
        }
        setVerticesHaveChanged(true);
    }
}
