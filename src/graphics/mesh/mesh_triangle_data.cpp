#include "mesh_triangle_data.h"
#include "../../config.h"
#include "../../controller.h"
#include "../../lib/Miniball.hpp"
#include "../../metrics.h"
#include "../opengl_native_or_replacement.h"

namespace bricksim::mesh {

    void TriangleData::initBuffers(const std::vector<MeshInstance>& instances) {
        controller::executeOpenGL([&]() {
            //VAO
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            //vertexVbo
            glGenBuffers(1, &vertexVBO);
            glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
            size_t vertex_size = sizeof(TriangleVertex);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * vertex_size, &(vertices[0]), GL_STATIC_DRAW);
            metrics::vramUsageBytes += vertices.size() * vertex_size;

            // position attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)nullptr);
            // normal attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)offsetof(TriangleVertex, normal));

            //instanceVbo
            auto instancesArray = generateInstancesArray(instances);

            glGenBuffers(1, &instanceVBO);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            size_t instanceSize = sizeof(TriangleInstance);
            instanceCount = instances.size();
            lastInstanceBufferSize = instanceCount;
            glBufferData(GL_ARRAY_BUFFER, instanceCount * instanceSize, &instancesArray[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, instanceSize, (void*)offsetof(TriangleInstance, diffuseColor));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, instanceSize, (void*)offsetof(TriangleInstance, ambientFactor));
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, instanceSize, (void*)offsetof(TriangleInstance, specularBrightness));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, instanceSize, (void*)offsetof(TriangleInstance, shininess));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, instanceSize, (void*)offsetof(TriangleInstance, idColor));
            for (int j = 0; j < 4; ++j) {
                glEnableVertexAttribArray(7 + j);
                glVertexAttribPointer(7 + j, 4, GL_FLOAT, GL_FALSE, instanceSize,
                                      (void*)(offsetof(TriangleInstance, transformation) + 4 * j * sizeof(float)));
            }

            for (int i = 2; i < 11; ++i) {
                glVertexAttribDivisor(i, 1);
            }

            //ebo
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &(indices[0]), GL_STATIC_DRAW);
            metrics::vramUsageBytes += sizeof(unsigned int) * indices.size();

            if (config::get(config::DELETE_VERTEX_DATA_AFTER_UPLOADING)) {
                dataAlreadyDeleted = true;
                uploadedVertexCount = vertices.size();
                uploadedIndexCount = indices.size();
                metrics::memorySavedByDeletingVertexData += vertices.capacity() * sizeof(TriangleVertex);
                metrics::memorySavedByDeletingVertexData += indices.capacity() * sizeof(unsigned int);
                vertices.clear();
                indices.clear();
                vertices.shrink_to_fit();
                indices.shrink_to_fit();
            }
        });
    }

    void TriangleData::freeBuffers() {
        controller::executeOpenGL([&]() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &vertexVBO);
            glDeleteBuffers(1, &instanceVBO);
            glDeleteBuffers(1, &EBO);
        });
    }

    void TriangleData::draw(const std::optional<InstanceRange>& sceneLayerInstanceRange) {
        if (sceneLayerInstanceRange.has_value() && sceneLayerInstanceRange->count > 0 && getIndexCount() > 0) {
            glBindVertexArray(VAO);
            graphics::opengl_native_or_replacement::drawElementsInstancedBaseInstance(
                    GL_TRIANGLES, getIndexCount(), GL_UNSIGNED_INT, nullptr, sceneLayerInstanceRange->count, sceneLayerInstanceRange->start,
                    instanceVBO, instanceCount * sizeof(TriangleInstance), sizeof(TriangleInstance));
        }
    }
    void TriangleData::rewriteInstanceBuffer(const std::vector<MeshInstance>& instances) {
        instanceCount = instances.size();
        controller::executeOpenGL([&]() {
            size_t newBufferSize = sizeof(TriangleInstance) * instanceCount;
            metrics::vramUsageBytes -= lastInstanceBufferSize;
            metrics::vramUsageBytes += newBufferSize;
            lastInstanceBufferSize = newBufferSize;
            auto instancesArray = generateInstancesArray(instances);
            size_t instance_size = sizeof(TriangleInstance);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);
        });
    }
    TriangleData::TriangleData(const ldr::ColorReference& color) :
        color(color) {
    }

    std::unique_ptr<TriangleInstance[], std::default_delete<TriangleInstance[]>> TriangleData::generateInstancesArray(const std::vector<MeshInstance>& instances) {
        auto instancesArray = std::make_unique<TriangleInstance[]>(instances.size());
        unsigned int arr_cursor = 0;
        if (color.get()->code == ldr::color_repo::INSTANCE_DUMMY_COLOR_CODE) {
            for (auto& instance: instances) {
                instancesArray[arr_cursor].transformation = glm::transpose(instance.transformation * constants::LDU_TO_OPENGL);
                instancesArray[arr_cursor].setColor(instance.color);
                instancesArray[arr_cursor].idColor = color::convertIntToColorVec3(instance.elementId);
                arr_cursor++;
            }
        } else {
            TriangleInstance inst{};
            inst.setColor(color);
            std::fill_n(instancesArray.get(), instances.size(), inst);
            for (auto& instance: instances) {
                instancesArray[arr_cursor].transformation = glm::transpose(instance.transformation * constants::LDU_TO_OPENGL);
                instancesArray[arr_cursor].idColor = color::convertIntToColorVec3(instance.elementId);
                arr_cursor++;
            }
        }
        return instancesArray;
    }

    size_t TriangleData::getVertexCount() const {
        return dataAlreadyDeleted ? uploadedVertexCount : vertices.size();
    }

    size_t TriangleData::getIndexCount() const {
        return dataAlreadyDeleted ? uploadedIndexCount : indices.size();
    }

    unsigned int TriangleData::addRawVertex(const TriangleVertex& vertex) {
        vertices.push_back(vertex);
        return vertices.size() - 1;
    }

    void TriangleData::addRawIndex(unsigned int index) {
        indices.push_back(index);
    }

    void TriangleData::addVertexWithIndex(const TriangleVertex& vertex) {
        indices.push_back(vertices.size());
        vertices.push_back(vertex);
    }

    void TriangleData::fillVerticesForOuterDimensions(std::unique_ptr<float*[]>& coords, size_t& coordCursor) {
        for (auto& item: vertices) {
            coords[coordCursor] = &item.position[0];
            ++coordCursor;
        }
    }
}
