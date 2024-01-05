#pragma once

#include "../../graphics/mesh/mesh.h"
#include "windows.h"

namespace bricksim::gui::windows::mesh_inspector {
    namespace {
        void drawColorLabel(const ldr::ColorReference& colorRef);
        void drawTriangleVerticesTab(std::shared_ptr<mesh::Mesh>& mesh);
        void drawInstancesTab(std::shared_ptr<mesh::Mesh>& mesh);
        void drawGeneralTab(std::shared_ptr<mesh::Mesh>& mesh);
    }

    void draw(Data& data);
    void setCurrentlyInspectingMesh(std::shared_ptr<mesh::Mesh> mesh);
}
