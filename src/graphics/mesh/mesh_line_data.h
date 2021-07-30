#pragma once

#include "mesh_simple_classes.h"

namespace bricksim::mesh {
    class LineData {
    public:
        explicit LineData(unsigned int drawMode);
        void initBuffers(const std::vector<MeshInstance>& instances);
        void freeBuffers();
        void draw(const std::optional<InstanceRange>& sceneLayerInstanceRange);
        void addVertex(const LineVertex& vertex);
        void rewriteInstanceBuffer(const std::vector<glm::mat4>& instances);

    private:
        std::vector<LineVertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int vao, vertexVBO, instanceVBO, ebo;
        size_t instanceCount;
        const unsigned int drawMode;
    };
}
