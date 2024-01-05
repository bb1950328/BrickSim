#include "mesh_textured_triangle_data.h"
#include "../../config/read.h"
#include "../../controller.h"
#include "../../metrics.h"
#include "../opengl_native_or_replacement.h"

namespace bricksim::mesh {
    TexturedTriangleData::TexturedTriangleData(std::shared_ptr<graphics::Texture> texture) :
        texture(std::move(texture)) {}

    void TexturedTriangleData::initBuffers(const std::vector<TexturedTriangleInstance>& instances) {
        if (!vertices.empty()) {
            controller::executeOpenGL([&instances, this]() {
                initBuffersImpl(instances);
            });
        }
    }

    void TexturedTriangleData::initBuffersImpl(const std::vector<TexturedTriangleInstance>& instances) {
        //VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        //vertexVbo
        glGenBuffers(1, &vertexVBO);
        glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
        constexpr auto vertexSize = sizeof(TexturedTriangleVertex);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>((vertices.size() * vertexSize)), vertices.data(), GL_STATIC_DRAW);
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
        constexpr auto instanceSize = static_cast<GLsizei>(sizeof(TexturedTriangleInstance));
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instances.size()) * instanceSize, instances.data(), GL_STATIC_DRAW);
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

        if (config::get().graphics.deleteVertexDataAfterUploading) {
            verticesAlreadyDeleted = true;
            uploadedVertexCount = vertices.size();
            metrics::memorySavedByDeletingVertexData += vertices.capacity() * sizeof(TexturedTriangleVertex);
            vertices.clear();
            vertices.shrink_to_fit();
        }
    }

    void TexturedTriangleData::rewriteInstanceBuffer(const std::vector<TexturedTriangleInstance>& instances) {
        controller::executeOpenGL([this, &instances]() {
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>((instances.size() * sizeof(TexturedTriangleInstance))), instances.data(), GL_STATIC_DRAW);
            uploadedInstanceCount = instances.size();
        });
    }

    void TexturedTriangleData::freeBuffers() const {
        controller::executeOpenGL([this]() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &vertexVBO);
            glDeleteBuffers(1, &instanceVBO);
        });
    }

    void TexturedTriangleData::draw(const InstanceRange& sceneLayerInstanceRange) const {
        size_t vertexCount = getVertexCount();
        if (vertexCount > 0) {
            texture->bind();
            glBindVertexArray(VAO);
            graphics::opengl_native_or_replacement::drawArraysInstancedBaseInstance(GL_TRIANGLES,
                                                                                    0,
                                                                                    static_cast<GLsizei>(vertexCount),
                                                                                    static_cast<GLsizei>(sceneLayerInstanceRange.count),
                                                                                    sceneLayerInstanceRange.start,
                                                                                    instanceVBO,
                                                                                    static_cast<GLsizeiptr>((uploadedInstanceCount * sizeof(TexturedTriangleInstance))),
                                                                                    sizeof(TexturedTriangleInstance));
        }
    }

    size_t TexturedTriangleData::getVertexCount() const {
        return verticesAlreadyDeleted ? uploadedVertexCount : vertices.size();
    }

    void TexturedTriangleData::addVerticesForOuterDimensions(std::vector<glm::dvec3>& coords) const {
        for (auto& item: vertices) {
            coords.push_back(item.position);
        }
    }

    void TexturedTriangleData::addVertex(const TexturedTriangleVertex& vertex) {
        vertices.push_back(vertex);
    }

    TexturedTriangleData::TexturedTriangleData(TexturedTriangleData&& other) noexcept :
        texture(std::move(other.texture)),
        vertices(std::move(other.vertices)),
        VAO(other.VAO),
        vertexVBO(other.vertexVBO),
        instanceVBO(other.instanceVBO),
        verticesAlreadyDeleted(other.verticesAlreadyDeleted),
        uploadedVertexCount(other.uploadedVertexCount),
        uploadedInstanceCount(other.uploadedInstanceCount) {}

    TexturedTriangleData& TexturedTriangleData::operator=(TexturedTriangleData&& other) noexcept {
        texture = std::move(other.texture);
        vertices = std::move(other.vertices);
        VAO = other.VAO;
        vertexVBO = other.vertexVBO;
        instanceVBO = other.instanceVBO;
        verticesAlreadyDeleted = other.verticesAlreadyDeleted;
        uploadedVertexCount = other.uploadedVertexCount;
        uploadedInstanceCount = other.uploadedInstanceCount;
        return *this;
    }
}
