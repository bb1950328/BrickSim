#pragma once

#include "../../graphics/mesh/mesh_generated.h"
#include "../connection.h"
#include "../connector/cylindrical.h"
#include <cstdint>

namespace bricksim::connection::visualization {
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

    class SimpleGeneratedLineNode : public mesh::generated::GeneratedMeshNode {
    protected:
        const SimpleLineColor lineColor;

        virtual void addToMesh(mesh::LineData& lineData, const glm::vec3& color) = 0;

    public:
        SimpleGeneratedLineNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor);
        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };

    class LineSunNode : public SimpleGeneratedLineNode {
    private:
        const bool inverted;
        const CylindricalShapeType shape;
        std::vector<glm::vec2> generateShape() const;

    protected:
        void addToMesh(mesh::LineData& lineData, const glm::vec3& color) override;

    public:
        LineSunNode(const std::shared_ptr<Node>& parent, SimpleLineColor color, bool inverted, CylindricalShapeType shape);

        std::string getDescription() override;

        mesh_identifier_t getMeshIdentifier() const override;
    };

    class PointMarkerNode : public SimpleGeneratedLineNode {
    protected:
        void addToMesh(mesh::LineData& lineData, const glm::vec3& color) override;

    public:
        PointMarkerNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor);
        mesh_identifier_t getMeshIdentifier() const override;
        std::string getDescription() override;
    };

    class LineUVSphereNode : public SimpleGeneratedLineNode {
        static constexpr int steps = 12;

        glm::vec3 getPointOnSphere(int iLat, int iLon);

    protected:
        void addToMesh(mesh::LineData& lineData, const glm::vec3& color) override;

    public:
        LineUVSphereNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
    };

    /**
     * Cylinder axis is along the Y axis
     */
    class LineCylinderNode : public SimpleGeneratedLineNode {
        static constexpr int steps = 12;

    protected:
        void addToMesh(mesh::LineData& lineData, const glm::vec3& color) override;

    public:
        LineCylinderNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
    };

    class LineBoxNode : public SimpleGeneratedLineNode {
    protected:
        void addToMesh(mesh::LineData& lineData, const glm::vec3& color) override;

    public:
        LineBoxNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
    };
}
