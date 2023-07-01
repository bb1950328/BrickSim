#pragma once

#include "data.h"
#include <memory>
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
        virtual Vertex* writeVertices(Vertex* firstVertexLocation, coord_t viewportSize) = 0;//todo change firstVertexLocation to non-const reference to an iterator and return type to void
        virtual ~Element();
    };
}
