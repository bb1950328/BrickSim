#include "element.h"

namespace bricksim::overlay2d {
    Element::Element() = default;

    void Element::setVerticesHaveChanged(bool value) {
        verticesHaveChanged = value;
    }

    bool Element::haveVerticesChanged() const {
        return verticesHaveChanged;
    }
    Element::~Element() = default;
}
