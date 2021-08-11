#pragma once

#include "element_tree.h"
#include "graphics/mesh/mesh_generated.h"
#include "graphics/scene.h"
#include "helpers/debug_nodes.h"
#include "helpers/ray.h"
#include "helpers/util.h"
#include "types.h"
#include <array>
#include <glm/glm.hpp>

namespace bricksim {
    class Editor;
}

namespace bricksim::transform_gizmo {

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
    protected:
        Ray3 calculateAxisRay(int axis);
        glm::mat4 startGizmoRelTransformation;
        glm::mat4 startNodeRelTransformation;
        const glm::vec2 startMousePos;

    public:
        TransformOperation(TransformGizmo& gizmo, const glm::vec2& startMousePos);

        TransformGizmo& gizmo;
        virtual void update(const glm::vec2& mouseDelta) = 0;
        virtual void cancel();
        virtual constexpr TransformType getType() = 0;

        virtual ~TransformOperation();
    };

    class Translate1dOperation : public TransformOperation {
    public:
        Translate1dOperation(TransformGizmo& gizmo, const glm::vec2& startMousePos, int axis);
        void update(const glm::vec2& mouseDelta) override;
        constexpr TransformType getType() override;

    private:
        glm::vec3 getClosestPointOnTransformRay(const glm::svec2& mouseCoords);
        const Ray3 transformRay;
        glm::vec3 startPointOnTransformRay;
    };

    class Translate2dOperation : public TransformOperation {
        glm::vec3 startPointOnPlane;

    public:
        Translate2dOperation(TransformGizmo& gizmo, const glm::vec2& startMousePos, int axis);
        void update(const glm::vec2& mouseDelta) override;
        constexpr TransformType getType() override;
        Ray<3> planeNormal;
    };

    class RotateOperation : public TransformOperation {
        Ray3 axis;
        glm::vec3 startPointOnPlane;
        util::DecomposedTransformation startNodeTransfDecomposed;
        util::DecomposedTransformation startGizmoTransfDecomposed;

    public:
        RotateOperation(TransformGizmo& gizmo, const glm::vec2& startMousePos, int axis);
        void update(const glm::vec2& mouseDelta) override;
        constexpr TransformType getType() override;
    };

    class TG2DArrowNode : public etree::MeshNode {
    private:
    public:
        TG2DArrowNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent);
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) override;
        bool isDisplayNameUserEditable() const override;
        std::string getDescription() override;
    };

    class TGNode : public etree::Node {
        friend class TransformOperation;
        friend class Translate1dOperation;
        friend class Translate2dOperation;

    private:
        std::array<std::shared_ptr<mesh::generated::ArrowNode>, 3> translate1dArrows;
        std::array<std::shared_ptr<mesh::generated::QuarterTorusNode>, 3> rotateQuarterTori;
        std::array<std::shared_ptr<TG2DArrowNode>, 3> translate2dArrows;
        std::shared_ptr<mesh::generated::UVSphereNode> centerBall;
        PovState povState;

    public:
        explicit TGNode(const std::shared_ptr<etree::Node>& parent);
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
        friend class Translate2dOperation;
        friend class RotateOperation;

    private:
        std::shared_ptr<graphics::Scene> scene;
        Editor& editor;
        std::shared_ptr<TGNode> node;
        std::optional<glm::mat4> lastTransformation;
        PovState lastPovState;
        std::unique_ptr<TransformOperation> currentTransformationOperation = nullptr;
        glm::mat4 nodeRotation;
        glm::vec3 nodePosition;

        std::shared_ptr<etree::Node> currentlySelectedNode;

    public:
        explicit TransformGizmo(Editor& editor);
        TransformGizmo(const TransformGizmo&) = delete;
        TransformGizmo(TransformGizmo&&) = delete;
        TransformGizmo& operator=(const TransformGizmo&) = delete;
        TransformGizmo& operator=(TransformGizmo&&) = delete;
        void update();
        bool ownsNode(const std::shared_ptr<etree::Node>& node_);
        void startDrag(std::shared_ptr<etree::Node>& draggedNode, glm::svec2 initialCursorPos);
        void updateCurrentDragDelta(glm::svec2 totalDragDelta);
        void endDrag();
        virtual ~TransformGizmo();
    };
}
