

#ifndef BRICKSIM_TRANSFORM_GIZMO_H
#define BRICKSIM_TRANSFORM_GIZMO_H

#include <memory>
#include "scene.h"

class ArrowMeshNode : public etree::MeshNode {
public:
    ArrowMeshNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent);
    bool isTransformationUserEditable() const override;
    bool isDisplayNameUserEditable() const override;
    std::string getDescription() override;
    mesh_identifier_t getMeshIdentifier() const override;
    void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
    bool isColorUserEditable() const override;
};

class TransformGizmoNode : public etree::Node {
private:
    std::shared_ptr<ArrowMeshNode> translateArrows[3];
public:
    TransformGizmoNode(const std::shared_ptr<Node> &parent);
    void initArrows();
    bool isTransformationUserEditable() const override;
    bool isDisplayNameUserEditable() const override;
};

class TransformGizmo {
private:
    std::shared_ptr<Scene> scene;
    std::shared_ptr<TransformGizmoNode> transformGizmoNode;
public:
    TransformGizmo(std::shared_ptr<Scene> scene);
    void update();
    virtual ~TransformGizmo();
};

#endif //BRICKSIM_TRANSFORM_GIZMO_H
