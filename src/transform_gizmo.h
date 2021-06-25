#ifndef BRICKSIM_TRANSFORM_GIZMO_H
#define BRICKSIM_TRANSFORM_GIZMO_H

#include <glm/glm.hpp>
#include "types.h"
#include "helpers/ray.h"
#include "element_tree.h"
#include "graphics/mesh/mesh_generated.h"
#include "graphics/scene.h"
#include <array>

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

    enum TransformType {
        TRANSLATE_1D,
        TRANSLATE_2D,
        TRANSLATE_3D,
        ROTATE,
        NONE,
    };

    enum class RotationState {
        SELECTED_ELEMENT,
        WORLD,
    };

    class TransformGizmo;

    class TransformOperation {
    public:
        TransformGizmo& gizmo;

        virtual void update(const glm::vec2& mouseDelta) = 0;
        virtual void cancel() = 0;
        virtual constexpr TransformType getType() = 0;
        explicit TransformOperation(TransformGizmo &gizmo);
        virtual ~TransformOperation();
    };

    class Translate1dOperation : public TransformOperation {
    public:
        Translate1dOperation(TransformGizmo &gizmo, const glm::vec2 &startMousePos, int axis);
        void update(const glm::vec2 &mouseDelta) override;
        void cancel() override;
        constexpr TransformType getType() override;
    private:
        glm::vec3 getClosestPointOnTransformRay(const glm::svec2& mouseCoords);
        Ray3 calculateTransformRay(int axis);
        const Ray3 transformRay;
        glm::mat4 startNodeRelTransformation;
        glm::mat4 startGizmoRelTransformation;
        glm::vec3 startPointOnTransformRay;
        const glm::vec2 startMousePos;
    };

    class TG2DArrowNode : public etree::MeshNode {
    private:
    public:
        TG2DArrowNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent);
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) override;
        bool isDisplayNameUserEditable() const override;
        std::string getDescription() override;
    };

    class TGNode : public etree::Node {
    private:
        std::array<std::shared_ptr<mesh::generated::ArrowNode>, 3> translate1dArrows;
        std::array<std::shared_ptr<mesh::generated::QuarterTorusNode>, 3> rotateQuarterTori;
        std::array<std::shared_ptr<TG2DArrowNode>, 3> translate2dArrows;
        std::shared_ptr<mesh::generated::UVSphereNode> centerBall;
        PovState povState;
    public:
        explicit TGNode(const std::shared_ptr<etree::Node> &parent);
        void initElements();
        bool isTransformationUserEditable() const override;
        bool isDisplayNameUserEditable() const override;
        PovState getPovState() const;
        void setPovState(PovState newState);
        void updateTransformations();
        std::string getDescription() override;
        std::pair<TransformType, int> getTransformTypeAndAxis(std::shared_ptr<etree::Node>& node);
    };

    class TransformGizmo {
        friend class TransformOperation;
        friend class Translate1dOperation;
    private:
        std::shared_ptr<Scene> scene;
        std::shared_ptr<TGNode> node;
        std::shared_ptr<mesh::generated::GeneratedMeshNode> debugNode;
        std::optional<glm::mat4> lastTransformation;
        PovState lastState;
        std::unique_ptr<TransformOperation> currentTransformationOperation = nullptr;
        glm::mat4 nodeRotation;
        glm::vec3 nodePosition;

        std::shared_ptr<etree::Node> currentlySelectedNode;
    public:
        explicit TransformGizmo(std::shared_ptr<Scene> scene);
        void update();
        bool ownsNode(const std::shared_ptr<etree::Node>& node_);
        void startDrag(std::shared_ptr<etree::Node> &draggedNode, glm::svec2 initialCursorPos);
        void updateCurrentDragDelta(glm::svec2 totalDragDelta);
        void endDrag();
        virtual ~TransformGizmo();
    };
}
#endif //BRICKSIM_TRANSFORM_GIZMO_H
