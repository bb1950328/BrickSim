#pragma once

#include "../../types.h"
#include "data.h"
#include "element.h"

namespace bricksim::overlay2d {
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
}
