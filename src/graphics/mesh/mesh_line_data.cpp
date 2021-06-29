#include "mesh_line_data.h"
#include "../../controller.h"
#include "../../metrics.h"
#include <glad/glad.h>

namespace bricksim::mesh {
    void LineData::initBuffers(const std::vector<MeshInstance>& instances) {
        controller::executeOpenGL([this, &instances]() {
            //vao
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            //vertexVbo
            glGenBuffers(1, &vertexVBO);
            glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
            size_t vertex_size = sizeof(LineVertex);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * vertex_size, &(vertices[0]), GL_STATIC_DRAW);
            metrics::vramUsageBytes += vertices.size() * vertex_size;

            // position attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void*)nullptr);
            // color attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)offsetof(LineVertex, color));

            //instanceVbo
            auto* instancesArray = new glm::mat4[instances.size()];//todo smart pointer/array
            for (int i = 0; i < instances.size(); ++i) {
                instancesArray[i] = glm::transpose(instances[i].transformation * constants::LDU_TO_OPENGL);
            }

            glGenBuffers(1, &instanceVBO);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            size_t instance_size = sizeof(glm::mat4);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);

            for (int j = 2; j < 6; ++j) {
                glEnableVertexAttribArray(j);
                glVertexAttribPointer(j, 4, GL_FLOAT, GL_FALSE, instance_size, (void*)(4 * (j - 2) * sizeof(float)));
                glVertexAttribDivisor(j, 1);
            }

            //ebo
            glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &(indices)[0], GL_STATIC_DRAW);
            metrics::vramUsageBytes += sizeof(unsigned int) * indices.size();

            delete[] instancesArray;
        });
    }

    void LineData::freeBuffers() {
        controller::executeOpenGL([this]() {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vertexVBO);
            glDeleteBuffers(1, &instanceVBO);
            glDeleteBuffers(1, &ebo);
        });
    }

    void LineData::draw(const std::optional<InstanceRange>& sceneLayerInstanceRange) {
        if (sceneLayerInstanceRange.has_value() && sceneLayerInstanceRange->count > 0 && !indices.empty()) {
            glBindVertexArray(vao);
            glDrawElementsInstancedBaseInstance(drawMode, indices.size(), GL_UNSIGNED_INT, nullptr, sceneLayerInstanceRange->count, sceneLayerInstanceRange->start);
        }
    }

    void LineData::addVertex(const LineVertex& vertex) {
        for (int i = (int)vertices.size() - 1; i >= std::max((int)vertices.size() - 12, 0); --i) {
            if (vertex.position == vertices[i].position && vertex.color == vertices[i].color) {
                indices.push_back(i);
                return;
            }
        }
        indices.push_back(vertices.size());
        vertices.push_back(vertex);
    }

    void LineData::rewriteInstanceBuffer(const std::vector<glm::mat4>& instances) {
        controller::executeOpenGL([this, &instances]() {
            constexpr size_t instance_size = sizeof(glm::mat4);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &(instances[0]), GL_STATIC_DRAW);
        });
    }

    LineData::LineData(const unsigned int drawMode) :
        drawMode(drawMode) {}
}