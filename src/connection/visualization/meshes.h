#pragma once

#include "../../graphics/mesh/mesh_generated.h"
#include "../data.h"
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

    class LineSunNode : public mesh::generated::GeneratedMeshNode {
    private:
        const SimpleLineColor lineColor;
        const bool inverted;
        const CylindricalShapeType shape;
        std::vector<glm::vec2> generateShape() const;

    public:
        LineSunNode(const std::shared_ptr<Node>& parent, SimpleLineColor color, bool inverted, CylindricalShapeType shape);

        std::string getDescription() override;

        mesh_identifier_t getMeshIdentifier() const override;

        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed,
                       const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
    };
}
