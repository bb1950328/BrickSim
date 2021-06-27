#ifndef BRICKSIM_WINDOW_MESH_INSPECTOR_H
#define BRICKSIM_WINDOW_MESH_INSPECTOR_H

#include "windows.h"
#include "../../graphics/mesh/mesh.h"

namespace bricksim::gui::windows::mesh_inspector {
    namespace {
        void drawColorLabel(const ldr::ColorReference &colorRef);
        void drawTriangleVerticesTab(std::shared_ptr<mesh::Mesh> &mesh);
        void drawInstancesTab(std::shared_ptr<mesh::Mesh> &mesh);
        void drawGeneralTab(std::shared_ptr<mesh::Mesh> &mesh);
    }
    void draw(Data& data);
    void setCurrentlyInspectingMesh(std::shared_ptr<mesh::Mesh> mesh);
}

#endif //BRICKSIM_WINDOW_MESH_INSPECTOR_H
