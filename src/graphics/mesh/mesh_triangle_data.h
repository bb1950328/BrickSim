#pragma once

#include "mesh_simple_classes.h"

namespace bricksim::mesh {
    class TriangleData {
    public:
        explicit TriangleData(const ldr::ColorReference& color);
        TriangleData(const TriangleData&) = delete;
        TriangleData(TriangleData&& other) noexcept;
        TriangleData& operator=(const TriangleData&) = delete;
        TriangleData& operator=(TriangleData&& other) noexcept;
        void initBuffers(const std::vector<MeshInstance>& instances);
        void freeBuffers() const;
        void draw(const std::optional<InstanceRange>& sceneLayerInstanceRange) const;
        unsigned int addRawVertex(const TriangleVertex& vertex);
        void addRawIndex(unsigned int index);
        void addVertexWithIndex(const TriangleVertex& vertex);
        void rewriteInstanceBuffer(const std::vector<MeshInstance>& instances);
        [[nodiscard]] size_t getVertexCount() const;
        [[nodiscard]] size_t getIndexCount() const;
        void addVerticesForOuterDimensions(std::vector<glm::dvec3>& coords) const;

        [[nodiscard]] bool isDataAlreadyDeleted() const;
        [[nodiscard]] const std::vector<TriangleVertex>& getVertices() const;
        [[nodiscard]] const std::vector<unsigned int>& getIndices() const;

    private:
        ldr::ColorReference color;
        std::vector<TriangleVertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int VAO;
        unsigned int vertexVBO;
        unsigned int instanceVBO;
        unsigned int EBO;
        std::vector<TriangleInstance> generateInstancesArray(const std::vector<MeshInstance>& instances) const;

        size_t instanceCount;
        size_t lastInstanceBufferSize = 0;
        bool dataAlreadyDeleted = false;
        size_t uploadedVertexCount;
        size_t uploadedIndexCount;
        void initBuffersImpl(const std::vector<MeshInstance>& instances);
    };
}
