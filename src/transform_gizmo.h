

#ifndef BRICKSIM_TRANSFORM_GIZMO_H
#define BRICKSIM_TRANSFORM_GIZMO_H

#include <memory>
#include "scene.h"
#include "generated_mesh.h"



class TransformGizmoNode : public etree::Node {
private:
    std::shared_ptr<generated_mesh::ArrowNode> translateArrows[3];
    std::shared_ptr<generated_mesh::QuarterTorusNode> rotateTori[3];
    std::shared_ptr<generated_mesh::UVSphereNode> centerBall;
public:
    TransformGizmoNode(const std::shared_ptr<Node> &parent);
    void initElements();
    bool isTransformationUserEditable() const override;
    bool isDisplayNameUserEditable() const override;
};

class TransformGizmo {
private:
    std::shared_ptr<Scene> scene;
    std::shared_ptr<TransformGizmoNode> node;
    std::optional<glm::mat4> lastTransformation;
public:
    TransformGizmo(std::shared_ptr<Scene> scene);
    void update();
    virtual ~TransformGizmo();
};

#endif //BRICKSIM_TRANSFORM_GIZMO_H
