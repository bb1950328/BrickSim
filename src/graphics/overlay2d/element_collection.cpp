#include "element_collection.h"
#include "../../controller.h"
#include <palanteer.h>

namespace bricksim::overlay2d {
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
                        vertices.erase(vertices.begin() + range.start, vertices.begin() + range.start + range.count);
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
                }
                if (!elementIsDeleted) {
                    const auto firstVertex = vertices.begin() + range.start;
                    auto it = firstVertex;
                    elem->writeVertices(it, viewportSize);
                    const auto effectiveVertexCount = std::distance(firstVertex, it);
                    assert(effectiveVertexCount == range.count && "Element::writeVertices() must increment vertex pointer by vertexCount");
                }
                if (!needToAppendAtEnd && !needToRewriteEverything) {
                    changedRanges.push_back(range);
                }
            }
            controller::executeOpenGL([this, needToRewriteEverything, &changedVerticesCount, &changedRanges]() {
                //todo update metrics::vramUsageBytes
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                if (needToRewriteEverything) {
                    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>((vertices.size() * sizeof(Vertex))), vertices.data(), GL_STATIC_DRAW);
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

    ElementCollection::ElementCollection() {
        // NOLINT(cppcoreguidelines-pro-type-member-init)
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
}
