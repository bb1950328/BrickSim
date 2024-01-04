#pragma once

#include "data.h"
#include <memory>
#include <vector>

namespace bricksim::overlay2d {
    class Element : private std::enable_shared_from_this<Element> {
    private:
        bool verticesHaveChanged = false;

    public:
        explicit Element();
        bool haveVerticesChanged() const;
        void setVerticesHaveChanged(bool value);
        virtual bool isPointInside(coord_t point) = 0;
        virtual unsigned int getVertexCount() = 0;
        virtual void writeVertices(std::vector<Vertex>::iterator& buffer, coord_t viewportSize) = 0;
        virtual ~Element();
    };
}
