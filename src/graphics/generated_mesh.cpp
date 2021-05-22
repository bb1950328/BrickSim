#include <glm/gtx/transform.hpp>
#include "generated_mesh.h"

namespace generated_mesh {

    mesh_identifier_t UVSphereNode::getMeshIdentifier() const {
        return constants::MESH_ID_UV_SPHERE;
    }

    UVSphereNode::UVSphereNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent) : GeneratedMeshNode(color, parent) {}

    void UVSphereNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {

        const auto &color = ldr_color_repo::getInstanceDummyColor();
        auto northPoleIndex = mesh->addRawTriangleVertex(color, TriangleVertex{glm::vec4(RADIUS, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)});
        auto southPoleIndex = mesh->addRawTriangleVertex(color, TriangleVertex{glm::vec4(-RADIUS, 0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)});
        auto firstMainIndex = mesh->getNextVertexIndex(color);

        float latDeltaRadians = M_PI / DIVISIONS;
        float lonDeltaRadians = 2.0f * M_PI / DIVISIONS;

        //to swap second and third index without branches
        auto secondIdxDelta = windingInversed ? +1 : 0;
        auto thirdIdxDelta = windingInversed ? -1 : 0;

        //todo this can be more efficient because the coordinates are 8 times the same just with different signs (but will be more difficult to get the indexes right)
        for (uint16_t iLat = 1; iLat < DIVISIONS; ++iLat) {
            float latLineRadius = std::sin(iLat * latDeltaRadians) * RADIUS;
            float x = std::cos(iLat * latDeltaRadians) * RADIUS;
            for (uint16_t iLon = 0; iLon < DIVISIONS; ++iLon) {
                float y = std::sin(iLon * lonDeltaRadians) * latLineRadius;
                float z = std::cos(iLon * lonDeltaRadians) * latLineRadius;
                mesh->addRawTriangleVertex(color, TriangleVertex{glm::vec4(x, y, z, 1.0f), glm::vec3(x, y, z) * (1 / RADIUS)});
            }
        }

        auto lastMainIndex = mesh->getNextVertexIndex(color) - 1;

        for (int i1 = 0, i2 = 1; i1 < DIVISIONS; ++i1, i2 = (i1 + 1) % DIVISIONS) {
            //north cap
            mesh->addRawTriangleIndex(color, northPoleIndex);
            mesh->addRawTriangleIndex(color, firstMainIndex + i2 + secondIdxDelta);
            mesh->addRawTriangleIndex(color, firstMainIndex + i1 + thirdIdxDelta);

            //south cap
            mesh->addRawTriangleIndex(color, southPoleIndex);
            mesh->addRawTriangleIndex(color, lastMainIndex - i2 + secondIdxDelta);
            mesh->addRawTriangleIndex(color, lastMainIndex - i1 + thirdIdxDelta);

            for (int j1 = 0, j2 = 1; j1 < DIVISIONS - 2; ++j1, ++j2) {
                mesh->addRawTriangleIndex(color, firstMainIndex + j1 * DIVISIONS + i1);
                mesh->addRawTriangleIndex(color, firstMainIndex + j1 * DIVISIONS + i2 + secondIdxDelta);
                mesh->addRawTriangleIndex(color, firstMainIndex + j2 * DIVISIONS + i2 + thirdIdxDelta);

                mesh->addRawTriangleIndex(color, firstMainIndex + j2 * DIVISIONS + i2);
                mesh->addRawTriangleIndex(color, firstMainIndex + j2 * DIVISIONS + i1 + secondIdxDelta);
                mesh->addRawTriangleIndex(color, firstMainIndex + j1 * DIVISIONS + i1 + thirdIdxDelta);
            }
        }
    }

    std::string UVSphereNode::getDescription() {
        return "UV Sphere";
    }

    GeneratedMeshNode::GeneratedMeshNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent) : MeshNode(color, parent) {}

    bool GeneratedMeshNode::isDisplayNameUserEditable() const {
        return false;
    }

    bool GeneratedMeshNode::isColorUserEditable() const {
        return false;
    }

    bool GeneratedMeshNode::isTransformationUserEditable() const {
        return false;
    }

    ArrowNode::ArrowNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent) : GeneratedMeshNode(color, parent) {}

    std::string ArrowNode::getDescription() {
        return "Arrow";
    }

    mesh_identifier_t ArrowNode::getMeshIdentifier() const {
        return constants::MESH_ID_ARROW;
    }

    void ArrowNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {
        glm::vec3 backCenter(0.0f, 0.0f, 0.0f);
        glm::vec3 beforeTipCenter(0.75f, 0.0f, 0.0f);
        glm::vec3 tip(1.0f, 0.0f, 0.0f);

        TriangleVertex backCoverVertex{
                glm::vec4(backCenter.x, backCenter.y + LINE_RADIUS, backCenter.z, 1.0f),
                glm::vec3(-1.0f, 0.0f, 0.0f)
        };

        TriangleVertex lineBackVertex{
                backCoverVertex.position,
                glm::vec3(0.0f, 1.0f, 0.0f)
        };
        TriangleVertex lineBeforeTipVertex{
                glm::vec4(beforeTipCenter.x, beforeTipCenter.y + LINE_RADIUS, beforeTipCenter.z, 1.0f),
                lineBackVertex.normal
        };

        TriangleVertex ringInnerVertex{
                lineBeforeTipVertex.position,
                backCoverVertex.normal
        };
        TriangleVertex ringOuterVertex{
                glm::vec4(beforeTipCenter.x, beforeTipCenter.y + TIP_RADIUS, beforeTipCenter.z, 1.0f),
                ringInnerVertex.normal
        };

        TriangleVertex tipEdgeVertex{
                ringOuterVertex.position,
                glm::normalize(glm::vec3(TIP_RADIUS, tip.x - beforeTipCenter.x, 0.0f))
        };

        TriangleVertex tipVertex{
                glm::vec4(tip, 1.0f),
                tipEdgeVertex.normal
        };

        std::vector<TriangleVertex> baseVertices{
                backCoverVertex,
                lineBackVertex,
                lineBeforeTipVertex,
                ringInnerVertex,
                ringOuterVertex,
                tipEdgeVertex,
                tipVertex,
        };
        unsigned long baseVertexCount = baseVertices.size();

        auto color = ldr_color_repo::getInstanceDummyColor();

        unsigned int firstIndex = mesh->getNextVertexIndex(color);

        for (uint16_t i = 0; i < NUM_CORNERS; ++i) {
            auto rotationMatrix = glm::rotate((float)(2 * M_PI * i / NUM_CORNERS), glm::vec3(1.0f, 0.0f, 0.0f));
            for (const auto &vertex : baseVertices) {
                mesh->addRawTriangleVertex(color, {vertex.position * rotationMatrix, glm::vec4(vertex.normal, 0.0f) * rotationMatrix});
            }
        }

        //to swap second and third index without branches
        auto secondIdxDelta = windingInversed ? +1 : 0;
        auto thirdIdxDelta = windingInversed ? -1 : 0;

        //special treatment for back circle
        for (uint16_t i1 = 1, i2 = 2; i2 < NUM_CORNERS; ++i2, ++i1) {
            mesh->addRawTriangleIndex(color, firstIndex);
            mesh->addRawTriangleIndex(color, firstIndex + baseVertexCount * i1 + secondIdxDelta);
            mesh->addRawTriangleIndex(color, firstIndex + baseVertexCount * i2 + thirdIdxDelta);
        }

        //       vtx0     vtx1
        // i1 -----+-------+
        //         |  \    |
        //         |    \  |
        // i2 -----+-------+
        for (uint16_t i0 = 0, i1 = 1; i0 < NUM_CORNERS; ++i0, i1 = (i0 + 1) % NUM_CORNERS) {
            for (int vtx0 = 0, vtx1 = 1; vtx1 < baseVertexCount; ++vtx0, ++vtx1) {
                mesh->addRawTriangleIndex(color, firstIndex + i0 * baseVertexCount + vtx0);
                mesh->addRawTriangleIndex(color, firstIndex + i0 * baseVertexCount + vtx1 + secondIdxDelta);
                mesh->addRawTriangleIndex(color, firstIndex + i1 * baseVertexCount + vtx1 + thirdIdxDelta);

                mesh->addRawTriangleIndex(color, firstIndex + i1 * baseVertexCount + vtx1);
                mesh->addRawTriangleIndex(color, firstIndex + i1 * baseVertexCount + vtx0 + secondIdxDelta);
                mesh->addRawTriangleIndex(color, firstIndex + i0 * baseVertexCount + vtx0 + thirdIdxDelta);
            }
        }
    }

    QuarterTorusNode::QuarterTorusNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent) : GeneratedMeshNode(color, parent) {}

    std::string QuarterTorusNode::getDescription() {
        return "Quarter Torus";
    }

    mesh_identifier_t QuarterTorusNode::getMeshIdentifier() const {
        return constants::MESH_ID_QUARTER_TORUS;
    }

    void QuarterTorusNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {
        const LdrColorReference &color = ldr_color_repo::getInstanceDummyColor();
        auto firstIndex = mesh->getNextVertexIndex(color);

        std::vector<TriangleVertex> baseVertices;
        float angleStep = 2 * M_PI / DIVISIONS;
        for (int i = 0; i < DIVISIONS; ++i) {
            float sinValue = std::sin(i * angleStep);
            float cosValue = std::cos(i * angleStep);
            glm::vec4 position(sinValue * TUBE_RADIUS, cosValue * TUBE_RADIUS + CENTER_TO_TUBE_RADIUS, 0.0f, 1.0f);
            glm::vec3 normal(-sinValue, -cosValue, 0.0f);
            baseVertices.push_back(TriangleVertex{position, normal});
        }

        constexpr float angleRatio = .25f;
        for (int i = 0; i <= DIVISIONS; ++i) {
            float angle = i * angleRatio * angleStep;
            auto rotation = glm::rotate(angle, glm::vec3(-1.0f, 0.0f, 0.0f));
            for (int j = 0; j < DIVISIONS; ++j) {
                mesh->addRawTriangleVertex(color, TriangleVertex{baseVertices[j].position * rotation, glm::vec4(baseVertices[j].normal, 0.0f) * rotation});
            }
        }

        //todo windingOrder
        for (int i1 = 0, i2 = 1; i1 < DIVISIONS; ++i1, ++i2) {
            for (int j1 = 0, j2 = 1; j1 < DIVISIONS; ++j1, j2 = (j1 + 1) % DIVISIONS) {
                mesh->addRawTriangleIndex(color, firstIndex + i1 * DIVISIONS + j1);
                mesh->addRawTriangleIndex(color, firstIndex + i1 * DIVISIONS + j2);
                mesh->addRawTriangleIndex(color, firstIndex + i2 * DIVISIONS + j2);

                mesh->addRawTriangleIndex(color, firstIndex + i2 * DIVISIONS + j2);
                mesh->addRawTriangleIndex(color, firstIndex + i2 * DIVISIONS + j1);
                mesh->addRawTriangleIndex(color, firstIndex + i1 * DIVISIONS + j1);
            }
        }

        if (WITH_ENDS) {
            auto capAFirstIndex = mesh->getNextVertexIndex(color);
            for (int j = 0; j < DIVISIONS; ++j) {
                mesh->addRawTriangleVertex(color, TriangleVertex{baseVertices[j].position, glm::vec3(0.0f, 0.0f, 1.0f)});
                if (j > 1) {
                    mesh->addRawTriangleIndex(color, capAFirstIndex);
                    mesh->addRawTriangleIndex(color, capAFirstIndex + j - 1);
                    mesh->addRawTriangleIndex(color, capAFirstIndex + j);
                }
            }

            auto capBFirstIndex = mesh->getNextVertexIndex(color);
            auto endRotation = glm::rotate(0.5f * float(M_PI), glm::vec3(1.0f, 0.0f, 0.0f));
            for (int j = 0; j < DIVISIONS; ++j) {
                mesh->addRawTriangleVertex(color, TriangleVertex{baseVertices[j].position * endRotation, glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) * endRotation});
                if (j > 1) {
                    mesh->addRawTriangleIndex(color, capBFirstIndex);
                    mesh->addRawTriangleIndex(color, capBFirstIndex + j - 1);
                    mesh->addRawTriangleIndex(color, capBFirstIndex + j);
                }
            }
        }
    }
}