#pragma once

#include "../ldr/files.h"
#include "mesh/mesh_simple_classes.h"
#include <glm/glm.hpp>

namespace bricksim::graphics::texmap_projection {
    glm::vec2 getPlanarUVCoord(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, glm::vec3 point);
    std::pair<std::pair<std::vector<unsigned int>, std::vector<mesh::TriangleVertex>>, std::vector<mesh::TexturedTriangleVertex>> splitTriangleBiggerThanTexturePlanar(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
}