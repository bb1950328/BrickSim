#pragma once

#include "../ldr/files.h"
#include "mesh/mesh_simple_classes.h"
#include <glm/glm.hpp>

namespace bricksim::graphics::texmap_projection {
    struct PolygonSplittingResult {
        std::vector<unsigned int> plainColorIndices;
        std::vector<mesh::TriangleVertex> plainColorVertices;
        std::vector<mesh::TexturedTriangleVertex> texturedVertices;
    };
    glm::vec2 getPlanarUVCoord(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, glm::vec3 point);
    PolygonSplittingResult splitPolygonBiggerThanTexturePlanar(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, const std::vector<glm::vec3>& points);
}