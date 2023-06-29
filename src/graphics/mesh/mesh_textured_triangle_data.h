#pragma once
#include "../texture.h"
#include "mesh_simple_classes.h"

namespace bricksim::mesh {
    class TexturedTriangleData {
    public:
        explicit TexturedTriangleData(std::shared_ptr<graphics::Texture> texture);
        TexturedTriangleData(const TexturedTriangleData&) = delete;
        TexturedTriangleData(TexturedTriangleData&& other) noexcept;
        TexturedTriangleData& operator=(const TexturedTriangleData&) = delete;
        TexturedTriangleData& operator=(TexturedTriangleData&& other) noexcept;
        void initBuffers(const std::vector<TexturedTriangleInstance>& instances);
        void rewriteInstanceBuffer(const std::vector<TexturedTriangleInstance>& instances);
        void freeBuffers() const;
        void draw(const InstanceRange& sceneLayerInstanceRange) const;
        [[nodiscard]] size_t getVertexCount() const;
        void fillVerticesForOuterDimensions(std::vector<const float*>& coords, size_t& coordCursor) const;
        void addVertex(const TexturedTriangleVertex& vertex);

    private:
        std::shared_ptr<graphics::Texture> texture;
        std::vector<TexturedTriangleVertex> vertices;
        unsigned int VAO;
        unsigned int vertexVBO;
        unsigned int instanceVBO;

        bool verticesAlreadyDeleted = false;
        size_t uploadedVertexCount;
        size_t uploadedInstanceCount;
        void initBuffersImpl(const std::vector<TexturedTriangleInstance>& instances);
    };
}
