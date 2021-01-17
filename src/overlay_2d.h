// overlay_2d.h
// Created by bab21 on 26.12.20.
//

#ifndef BRICKSIM_OVERLAY_2D_H
#define BRICKSIM_OVERLAY_2D_H

#include <glm/glm.hpp>
#include <vector>
#include "types.h"

class Overlay2dVertex {
    glm::vec2 position;
    glm::vec3 color;
    element_id_t elementId;
};

class Overlay2d {
    virtual element_id_t rebuild(element_id_t firstElementId) = 0;//return is last used element id
    virtual void click(element_id_t elementId) = 0;
};

class Overlay2dCollection {
private:
    std::vector<Overlay2d> overlays;
    element_id_t firstElementId;
    std::vector<element_id_t> lastElementIds;
public:
    void rebuild(element_id_t firstElementId);
    bool clickEvent(element_id_t elementId);
};

#endif //BRICKSIM_OVERLAY_2D_H
