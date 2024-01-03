#include "mesh_line_data.h"
#include "../../config/read.h"
#include "../../controller.h"
#include "../../metrics.h"
#include "../opengl_native_or_replacement.h"
#include <glad/glad.h>

namespace bricksim::mesh {
    void LineData::initBuffers(const std::vector<glm::mat4>& instances) {
        controller::executeOpenGL([this, &instances]() {
            //VAO
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            //vertexVbo
            glGenBuffers(1, &vertexVBO);
            glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
            size_t vertex_size = sizeof(LineVertex);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>((vertices.size() * vertex_size)), vertices.data(), GL_STATIC_DRAW);
            metrics::vramUsageBytes += vertices.size() * vertex_size;

            // position attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(vertex_size), (void*)nullptr);
            // color attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(vertex_size), (void*)offsetof(LineVertex, color));

            //instanceVbo
            instanceCount = instances.size();
            glGenBuffers(1, &instanceVBO);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>((instanceCount * sizeof(glm::mat4))), instances.data(), GL_STATIC_DRAW);

            for (int j = 2; j < 6; ++j) {
                glEnableVertexAttribArray(j);
                glVertexAttribPointer(j, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(4 * (j - 2) * sizeof(float)));
                glVertexAttribDivisor(j, 1);
            }

            //ebo
            glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>((sizeof(unsigned int) * indices.size())), indices.data(), GL_STATIC_DRAW);
            metrics::vramUsageBytes += sizeof(unsigned int) * indices.size();

            if (config::get().graphics.deleteVertexDataAfterUploading) {
                dataAlreadyDeleted = true;
                uploadedVertexCount = vertices.size();
                uploadedIndexCount = indices.size();
                metrics::memorySavedByDeletingVertexData += vertices.capacity() * sizeof(LineVertex);
                metrics::memorySavedByDeletingVertexData += indices.capacity() * sizeof(unsigned int);
                vertices.clear();
                indices.clear();
                vertices.shrink_to_fit();
                indices.shrink_to_fit();
            }
        });
    }

    void LineData::freeBuffers() const {
        controller::executeOpenGL([this]() {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vertexVBO);
            glDeleteBuffers(1, &instanceVBO);
            glDeleteBuffers(1, &ebo);
        });
    }

    void LineData::draw(const std::optional<InstanceRange>& sceneLayerInstanceRange) const {
        if (sceneLayerInstanceRange.has_value() && sceneLayerInstanceRange->count > 0 && getIndexCount() > 0) {
            glBindVertexArray(vao);
            graphics::opengl_native_or_replacement::drawElementsInstancedBaseInstance(drawMode,
                                                                                      static_cast<GLsizei>(getIndexCount()),
                                                                                      GL_UNSIGNED_INT,
                                                                                      nullptr,
                                                                                      static_cast<GLsizei>(sceneLayerInstanceRange->count),
                                                                                      sceneLayerInstanceRange->start,
                                                                                      instanceVBO,
                                                                                      static_cast<GLsizeiptr>(instanceCount * sizeof(glm::mat4)),
                                                                                      sizeof(glm::mat4));
        }
    }

    void LineData::addVertex(const LineVertex& vertex) {
        for (int i = (int)vertices.size() - 1; i >= std::max((int)vertices.size() - 12, 0); --i) {
            if (vertex.position == vertices[i].position && vertex.color == vertices[i].color) {
                indices.push_back(i);
                return;
            }
        }
        indices.push_back(static_cast<unsigned int>(vertices.size()));
        vertices.push_back(vertex);
    }

    void LineData::rewriteInstanceBuffer(const std::vector<glm::mat4>& instances) const {
        controller::executeOpenGL([this, &instances]() {
            constexpr size_t instance_size = sizeof(glm::mat4);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>((instances.size() * instance_size)), instances.data(), GL_STATIC_DRAW);
        });
    }

    LineData::LineData(const unsigned int drawMode) :
        drawMode(drawMode) {}
    size_t LineData::getVertexCount() const {
        return dataAlreadyDeleted ? uploadedVertexCount : vertices.size();
    }
    size_t LineData::getIndexCount() const {
        return dataAlreadyDeleted ? uploadedIndexCount : indices.size();
    }
}
