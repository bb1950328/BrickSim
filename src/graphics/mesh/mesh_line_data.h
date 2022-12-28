#pragma once

#include "mesh_simple_classes.h"

namespace bricksim::mesh {
    class LineData {
    public:
        explicit LineData(unsigned int drawMode);
        LineData(const LineData&) = delete;
        LineData& operator=(const LineData&) = delete;
        void initBuffers(const std::vector<glm::mat4>& instances);
        void freeBuffers() const;
        void draw(const std::optional<InstanceRange>& sceneLayerInstanceRange) const;
        void addVertex(const LineVertex& vertex);
        void rewriteInstanceBuffer(const std::vector<glm::mat4>& instances) const;

    private:
        std::vector<LineVertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int vao;
        unsigned int vertexVBO;
        unsigned int instanceVBO;
        unsigned int ebo;
        size_t instanceCount;
        const unsigned int drawMode;

        bool dataAlreadyDeleted = false;
        size_t uploadedVertexCount;
        size_t uploadedIndexCount;

        [[nodiscard]] size_t getVertexCount() const;
        [[nodiscard]] size_t getIndexCount() const;
    };
}
