#include "mesh_textured_triangle_data.h"
#include "../../config.h"
#include "../../controller.h"
#include "../../metrics.h"
#include "../opengl_native_or_replacement.h"
#include "../shaders.h"

namespace bricksim::mesh {

    TexturedTriangleData::TexturedTriangleData(std::shared_ptr<graphics::Texture> texture) :
        texture(std::move(texture)) {
    }
    void TexturedTriangleData::initBuffers(const std::vector<TexturedTriangleInstance>& instances) {
        if (!vertices.empty()) {
            controller::executeOpenGL([&]() {
                //VAO
                glGenVertexArrays(1, &VAO);
                glBindVertexArray(VAO);

                //vertexVbo
                glGenBuffers(1, &vertexVBO);
                glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
                constexpr auto vertexSize = sizeof(TexturedTriangleVertex);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * vertexSize, &vertices[0], GL_STATIC_DRAW);
                metrics::vramUsageBytes += vertices.size() * vertexSize;

                //position attribute
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(offsetof(TexturedTriangleVertex, position)));

                //texCoord attribute
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(offsetof(TexturedTriangleVertex, textureCoord)));

                //instanceVbo
                glGenBuffers(1, &instanceVBO);
                glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
                size_t instanceSize = sizeof(TexturedTriangleInstance);
                glBufferData(GL_ARRAY_BUFFER, instances.size() * instanceSize, &instances[0], GL_STATIC_DRAW);
                uploadedInstanceCount = instances.size();

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, instanceSize, (void*)offsetof(TexturedTriangleInstance, idColor));
                glVertexAttribDivisor(2, 1);

                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, instanceSize, (void*)(offsetof(TexturedTriangleInstance, transformation) + 0 * sizeof(float)));
                glVertexAttribDivisor(3, 1);
                glEnableVertexAttribArray(4);
                glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, instanceSize, (void*)(offsetof(TexturedTriangleInstance, transformation) + 4 * sizeof(float)));
                glVertexAttribDivisor(4, 1);
                glEnableVertexAttribArray(5);
                glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, instanceSize, (void*)(offsetof(TexturedTriangleInstance, transformation) + 8 * sizeof(float)));
                glVertexAttribDivisor(5, 1);
                glEnableVertexAttribArray(6);
                glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, instanceSize, (void*)(offsetof(TexturedTriangleInstance, transformation) + 12 * sizeof(float)));
                glVertexAttribDivisor(6, 1);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                if (config::get(config::DELETE_VERTEX_DATA_AFTER_UPLOADING)) {
                    verticesAlreadyDeleted = true;
                    uploadedVertexCount = vertices.size();
                    metrics::memorySavedByDeletingVertexData += vertices.capacity() * sizeof(TexturedTriangleVertex);
                    vertices.clear();
                    vertices.shrink_to_fit();
                }
            });
        }
    }

    void TexturedTriangleData::rewriteInstanceBuffer(const std::vector<TexturedTriangleInstance>& instances) {
        controller::executeOpenGL([&]() {
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(TexturedTriangleInstance), &instances[0], GL_STATIC_DRAW);
            uploadedInstanceCount = instances.size();
        });
    }
    void TexturedTriangleData::freeBuffers() {
        controller::executeOpenGL([&]() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &vertexVBO);
            glDeleteBuffers(1, &instanceVBO);
        });
    }
    void TexturedTriangleData::draw(const InstanceRange& sceneLayerInstanceRange) {
        size_t vertexCount = getVertexCount();
        if (vertexCount > 0) {
            texture->bind();
            glBindVertexArray(VAO);
            graphics::opengl_native_or_replacement::drawArraysInstancedBaseInstance(
                    GL_TRIANGLES, 0, vertexCount, sceneLayerInstanceRange.count, sceneLayerInstanceRange.start,
                    instanceVBO, uploadedInstanceCount * sizeof(TexturedTriangleInstance), sizeof(TexturedTriangleInstance));
        }
    }
    size_t TexturedTriangleData::getVertexCount() const {
        return verticesAlreadyDeleted ? uploadedVertexCount : vertices.size();
    }
    void TexturedTriangleData::fillVerticesForOuterDimensions(std::unique_ptr<const float*[]>& coords, size_t& coordCursor) const {
        for (auto& item: vertices) {
            coords[coordCursor] = &item.position[0];
            ++coordCursor;
        }
    }
    void TexturedTriangleData::addVertex(const TexturedTriangleVertex& vertex) {
        vertices.push_back(vertex);
    }
}