

#ifndef BRICKSIM_TRANSFORM_GIZMO_H
#define BRICKSIM_TRANSFORM_GIZMO_H

#include <memory>
#include "scene.h"

class TransformGizmo {
private:
    std::shared_ptr<Scene> scene;

    std::shared_ptr<overlay2d::ArrowElement> arrows[3];
    std::shared_ptr<overlay2d::RegularPolygonElement> centerDot;
public:
    TransformGizmo(std::shared_ptr<Scene> scene);
    void update();
    virtual ~TransformGizmo();
};

#endif //BRICKSIM_TRANSFORM_GIZMO_H
