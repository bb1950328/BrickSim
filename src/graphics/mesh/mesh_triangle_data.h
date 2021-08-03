#pragma once

namespace bricksim::mesh {
    class TriangleData;
}

#include "mesh.h"

namespace bricksim::mesh {
    class TriangleData {
    public:
        TriangleData(const ldr::ColorReference& color);
        void initBuffers(const std::vector<MeshInstance>& instances);
        void freeBuffers();
        void draw(const std::optional<InstanceRange>& sceneLayerInstanceRange);
        unsigned int addRawVertex(const TriangleVertex& vertex);
        void addRawIndex(unsigned int index);
        void addVertexWithIndex(const TriangleVertex& vertex);
        void rewriteInstanceBuffer(const std::vector<MeshInstance>& instances);
        size_t getVertexCount() const;
        size_t getIndexCount() const;
        void fillVerticesForOuterDimensions(std::unique_ptr<float*[]>& coords, size_t& coordCursor);

    private:
        ldr::ColorReference color;
        std::vector<TriangleVertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int VAO, vertexVBO, instanceVBO, EBO;
        std::vector<TriangleInstance> generateInstancesArray(const std::vector<MeshInstance>& instances);

        size_t instanceCount;
        size_t lastInstanceBufferSize = 0;
        bool dataAlreadyDeleted = false;
        size_t uploadedVertexCount, uploadedIndexCount;
    };
}