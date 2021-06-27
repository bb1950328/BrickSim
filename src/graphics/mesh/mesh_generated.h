#pragma once

#include <memory>
#include "../../element_tree.h"
#include "../../ldr/colors.h"
#include "../../types.h"
#include "mesh.h"

namespace bricksim::mesh::generated {

    class GeneratedMeshNode : public etree::MeshNode {
    public:
        GeneratedMeshNode(const ldr::ColorReference &color, const std::shared_ptr<Node> &parent);
        bool isDisplayNameUserEditable() const override;
        bool isTransformationUserEditable() const override;
        bool isColorUserEditable() const override;
    };

    class UVSphereNode : public GeneratedMeshNode {
    public:
        static constexpr float RADIUS = 0.5f;
        static constexpr uint16_t DIVISIONS = 12;
        UVSphereNode(const ldr::ColorReference &color, const std::shared_ptr<Node> &parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
    };

    class ArrowNode : public GeneratedMeshNode {
    public:
        static constexpr uint16_t NUM_CORNERS = 12;
        static constexpr float LINE_RADIUS = 0.03f;
        static constexpr float TIP_RADIUS = 3*LINE_RADIUS;
        ArrowNode(const ldr::ColorReference &color, const std::shared_ptr<Node> &parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
    };

    class QuarterTorusNode: public GeneratedMeshNode {
    public:
        static constexpr float CENTER_TO_TUBE_RADIUS = 0.5f;
        static constexpr float TUBE_RADIUS = 0.03f;
        static constexpr uint16_t DIVISIONS = 12;
        static constexpr bool WITH_ENDS = false;
        QuarterTorusNode(const ldr::ColorReference &color, const std::shared_ptr<Node> &parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
    };

    class CubeNode : public GeneratedMeshNode {
    public:
        CubeNode(const ldr::ColorReference &color, const std::shared_ptr<Node> &parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
    };
}

