#pragma once

#include "mesh_simple_classes.h"

namespace bricksim::mesh {
    class TriangleData {
    public:
        explicit TriangleData(const ldr::ColorReference& color);
        TriangleData(const TriangleData&) = delete;
        TriangleData& operator=(const TriangleData&) = delete;
        void initBuffers(const std::vector<MeshInstance>& instances);
        void freeBuffers();
        void draw(const std::optional<InstanceRange>& sceneLayerInstanceRange);
        unsigned int addRawVertex(const TriangleVertex& vertex);
        void addRawIndex(unsigned int index);
        void addVertexWithIndex(const TriangleVertex& vertex);
        void rewriteInstanceBuffer(const std::vector<MeshInstance>& instances);
        [[nodiscard]] size_t getVertexCount() const;
        [[nodiscard]] size_t getIndexCount() const;
        void fillVerticesForOuterDimensions(std::unique_ptr<const float*[]>& coords, size_t& coordCursor) const;

        [[nodiscard]] bool isDataAlreadyDeleted() const;
        [[nodiscard]] const std::vector<TriangleVertex>& getVertices() const;
        [[nodiscard]] const std::vector<unsigned int>& getIndices() const;

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