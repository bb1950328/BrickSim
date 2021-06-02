#ifndef BRICKSIM_MESH_LINE_DATA_H
#define BRICKSIM_MESH_LINE_DATA_H

#include "mesh_simple_classes.h"

namespace mesh {
    class LineData {
    public:
        explicit LineData(unsigned int drawMode);
        void initBuffers(const std::vector<MeshInstance>& instances);
        void freeBuffers();
        void draw(const std::optional<InstanceRange> &sceneLayerInstanceRange);
        void addVertex(const LineVertex &vertex);
        void rewriteInstanceBuffer(const std::vector<glm::mat4>& instances);
    private:
        std::vector<LineVertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int vao, vertexVBO, instanceVBO, ebo;
        const unsigned int drawMode;
    };
}

#endif //BRICKSIM_MESH_LINE_DATA_H
