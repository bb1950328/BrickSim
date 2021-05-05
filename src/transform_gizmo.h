

#ifndef BRICKSIM_TRANSFORM_GIZMO_H
#define BRICKSIM_TRANSFORM_GIZMO_H

#include <memory>
#include "scene.h"
#include "generated_mesh.h"

namespace transform_gizmo {

    /**
     * From which angle the transform gizmo is currently seen
     */
    enum class PovState {
        XNEG_YNEG_ZNEG = 0b000,
        XNEG_YNEG_ZPOS = 0b001,
        XNEG_YPOS_ZNEG = 0b010,
        XNEG_YPOS_ZPOS = 0b011,
        XPOS_YNEG_ZNEG = 0b100,
        XPOS_YNEG_ZPOS = 0b101,
        XPOS_YPOS_ZNEG = 0b110,
        XPOS_YPOS_ZPOS = 0b111,
    };

    enum class RotationState {
        SELECTED_ELEMENT,
        WORLD,
    };

    class TGNode : public etree::Node {
    private:
        std::shared_ptr<generated_mesh::ArrowNode> translateArrows[3];
        std::shared_ptr<generated_mesh::QuarterTorusNode> rotateTori[3];
        std::shared_ptr<generated_mesh::UVSphereNode> centerBall;
        PovState povState;
    public:
        explicit TGNode(const std::shared_ptr<etree::Node> &parent);
        void initElements();
        bool isTransformationUserEditable() const override;
        bool isDisplayNameUserEditable() const override;
        PovState getPovState() const;
        void setPovState(PovState newState);
        void updateTransformations();
    };

    class TransformGizmo {
    private:
        std::shared_ptr<Scene> scene;
        std::shared_ptr<TGNode> node;
        std::optional<glm::mat4> lastTransformation;
        PovState lastState;
    public:
        explicit TransformGizmo(std::shared_ptr<Scene> scene);
        void update();
        virtual ~TransformGizmo();
    };
}
#endif //BRICKSIM_TRANSFORM_GIZMO_H
