#pragma once

#include "../../element_tree.h"
#include "../../ldr/colors.h"
#include "../../types.h"
#include "mesh.h"
#include <memory>

namespace bricksim::mesh::generated {

    enum class TriangleLineMode {
        TRIANGLES_AND_LINES,
        ONLY_TRIANGLES,
        ONLY_LINES,
    };

    class GeneratedMeshNode : public etree::MeshNode {
    public:
        GeneratedMeshNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent);

        bool isDisplayNameUserEditable() const override;

        bool isTransformationUserEditable() const override;

        bool isColorUserEditable() const override;
    };

    class UVSphereNode : public GeneratedMeshNode {
    public:
        static constexpr float RADIUS = 0.5f;
        static constexpr uint16_t DIVISIONS = 12;
        UVSphereNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };

    class ArrowNode : public GeneratedMeshNode {
    public:
        static constexpr uint16_t NUM_CORNERS = 12;
        static constexpr float LINE_RADIUS = 0.03f;
        static constexpr float TIP_RADIUS = 3 * LINE_RADIUS;
        ArrowNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };

    class QuarterTorusNode : public GeneratedMeshNode {
    public:
        static constexpr float CENTER_TO_TUBE_RADIUS = 0.5f;
        static constexpr float TUBE_RADIUS = 0.03f;
        static constexpr uint16_t DIVISIONS = 12;
        static constexpr bool WITH_ENDS = false;
        QuarterTorusNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;

        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed,
                       const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };

    class CubeNode : public GeneratedMeshNode {
    public:
        CubeNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent);

        std::string getDescription() override;

        mesh_identifier_t getMeshIdentifier() const override;

        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed,
                       const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };

    /**
     * A cylinder with length 1 and radius 1
     * Coordinate origin is in center and start/end circles are in Z direction
     */
    class CylinderNode : public GeneratedMeshNode {
    public:
        static constexpr uint16_t NUM_CORNERS = 12;

        CylinderNode(const ldr::ColorReference& triangleColor, const std::shared_ptr<Node>& parent);

        std::string getDescription() override;

        mesh_identifier_t getMeshIdentifier() const override;

        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed,
                       const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };

    enum class SimpleLineColor {
        RED,
        GREEN,
        BLUE,
        CYAN,
        MAGENTA,
        YELLOW,
        WHITE,
        BLACK,
    };

    class LineSunNode : public GeneratedMeshNode {
    private:
        SimpleLineColor lineColor;

    public:
        static constexpr uint16_t NUM_CORNERS = 12;

        LineSunNode(const std::shared_ptr<Node>& parent, SimpleLineColor color);

        std::string getDescription() override;

        mesh_identifier_t getMeshIdentifier() const override;

        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed,
                       const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };

    class XYZLineNode : public GeneratedMeshNode {
    public:
        XYZLineNode(const std::shared_ptr<Node>& parent);

    private:
        std::string getDescription() override;

        mesh_identifier_t getMeshIdentifier() const override;

        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed,
                       const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };
}
