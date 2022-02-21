#pragma once
#include "../texture.h"
#include "mesh_simple_classes.h"

namespace bricksim::mesh {
    class TexturedTriangleData {
    public:
        explicit TexturedTriangleData(std::shared_ptr<graphics::Texture> texture);
        TexturedTriangleData(const TexturedTriangleData&) = delete;
        TexturedTriangleData& operator=(const TexturedTriangleData&) = delete;
        void initBuffers(const std::vector<TexturedTriangleInstance>& instances);
        void rewriteInstanceBuffer(const std::vector<TexturedTriangleInstance>& instances);
        void freeBuffers();
        void draw(const InstanceRange& sceneLayerInstanceRange);
        [[nodiscard]] size_t getVertexCount() const;
        void fillVerticesForOuterDimensions(std::vector<const float*>& coords, size_t& coordCursor) const;
        void addVertex(const TexturedTriangleVertex& vertex);
    private:
        std::shared_ptr<graphics::Texture> texture;
        std::vector<TexturedTriangleVertex> vertices;
        unsigned int VAO, vertexVBO, instanceVBO;

        bool verticesAlreadyDeleted = false;
        size_t uploadedVertexCount;
        size_t uploadedInstanceCount;
    };
}