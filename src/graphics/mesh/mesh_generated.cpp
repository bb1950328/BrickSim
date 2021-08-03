#include "mesh_generated.h"
#include <glm/gtx/transform.hpp>
#include <spdlog/spdlog.h>

namespace bricksim::mesh::generated {

    mesh_identifier_t UVSphereNode::getMeshIdentifier() const {
        return constants::MESH_ID_UV_SPHERE;
    }

    UVSphereNode::UVSphereNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent) :
        GeneratedMeshNode(color, parent) {}

    void UVSphereNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {
        const auto& color = ldr::color_repo::getInstanceDummyColor();
        auto& triangleData = mesh->getTriangleData(color);
        auto northPoleIndex = triangleData.addRawVertex(TriangleVertex{glm::vec4(RADIUS, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)});
        auto southPoleIndex = triangleData.addRawVertex(TriangleVertex{glm::vec4(-RADIUS, 0.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)});
        auto firstMainIndex = triangleData.getVertexCount();

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
                triangleData.addRawVertex(TriangleVertex{glm::vec4(x, y, z, 1.0f), glm::vec3(x, y, z) * (1 / RADIUS)});
            }
        }

        auto lastMainIndex = triangleData.getVertexCount() - 1;

        for (int i1 = 0, i2 = 1; i1 < DIVISIONS; ++i1, i2 = (i1 + 1) % DIVISIONS) {
            //north cap
            triangleData.addRawIndex(northPoleIndex);
            triangleData.addRawIndex(firstMainIndex + i2 + secondIdxDelta);
            triangleData.addRawIndex(firstMainIndex + i1 + thirdIdxDelta);

            //south cap
            triangleData.addRawIndex(southPoleIndex);
            triangleData.addRawIndex(lastMainIndex - i2 + secondIdxDelta);
            triangleData.addRawIndex(lastMainIndex - i1 + thirdIdxDelta);

            for (int j1 = 0, j2 = 1; j1 < DIVISIONS - 2; ++j1, ++j2) {
                triangleData.addRawIndex(firstMainIndex + j1 * DIVISIONS + i1);
                triangleData.addRawIndex(firstMainIndex + j1 * DIVISIONS + i2 + secondIdxDelta);
                triangleData.addRawIndex(firstMainIndex + j2 * DIVISIONS + i2 + thirdIdxDelta);

                triangleData.addRawIndex(firstMainIndex + j2 * DIVISIONS + i2);
                triangleData.addRawIndex(firstMainIndex + j2 * DIVISIONS + i1 + secondIdxDelta);
                triangleData.addRawIndex(firstMainIndex + j1 * DIVISIONS + i1 + thirdIdxDelta);
            }
        }
    }

    std::string UVSphereNode::getDescription() {
        return "UV Sphere";
    }

    GeneratedMeshNode::GeneratedMeshNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent) :
        MeshNode(color, parent) {}

    bool GeneratedMeshNode::isDisplayNameUserEditable() const {
        return false;
    }

    bool GeneratedMeshNode::isColorUserEditable() const {
        return false;
    }

    bool GeneratedMeshNode::isTransformationUserEditable() const {
        return false;
    }

    ArrowNode::ArrowNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent) :
        GeneratedMeshNode(color, parent) {}

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
                glm::vec3(-1.0f, 0.0f, 0.0f)};

        TriangleVertex lineBackVertex{
                backCoverVertex.position,
                glm::vec3(0.0f, 1.0f, 0.0f)};
        TriangleVertex lineBeforeTipVertex{
                glm::vec4(beforeTipCenter.x, beforeTipCenter.y + LINE_RADIUS, beforeTipCenter.z, 1.0f),
                lineBackVertex.normal};

        TriangleVertex ringInnerVertex{
                lineBeforeTipVertex.position,
                backCoverVertex.normal};
        TriangleVertex ringOuterVertex{
                glm::vec4(beforeTipCenter.x, beforeTipCenter.y + TIP_RADIUS, beforeTipCenter.z, 1.0f),
                ringInnerVertex.normal};

        TriangleVertex tipEdgeVertex{
                ringOuterVertex.position,
                glm::normalize(glm::vec3(TIP_RADIUS, tip.x - beforeTipCenter.x, 0.0f))};

        TriangleVertex tipVertex{
                glm::vec4(tip, 1.0f),
                tipEdgeVertex.normal};

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

        auto color = ldr::color_repo::getInstanceDummyColor();

        auto& triangleData = mesh->getTriangleData(color);

        unsigned int firstIndex = triangleData.getVertexCount();

        for (uint16_t i = 0; i < NUM_CORNERS; ++i) {
            auto rotationMatrix = glm::rotate((float)(2 * M_PI * i / NUM_CORNERS), glm::vec3(1.0f, 0.0f, 0.0f));
            for (const auto& vertex: baseVertices) {
                triangleData.addRawVertex({glm::vec4(vertex.position, 1.f) * rotationMatrix, glm::vec4(vertex.normal, 0.0f) * rotationMatrix});
            }
        }

        //to swap second and third index without branches
        auto secondIdxDelta = windingInversed ? +1 : 0;
        auto thirdIdxDelta = windingInversed ? -1 : 0;

        //special treatment for back circle
        for (uint16_t i1 = 1, i2 = 2; i2 < NUM_CORNERS; ++i2, ++i1) {
            triangleData.addRawIndex(firstIndex);
            triangleData.addRawIndex(firstIndex + baseVertexCount * i1 + secondIdxDelta);
            triangleData.addRawIndex(firstIndex + baseVertexCount * i2 + thirdIdxDelta);
        }

        //       vtx0     vtx1
        // i1 -----+-------+
        //         |  \    |
        //         |    \  |
        // i2 -----+-------+
        for (uint16_t i0 = 0, i1 = 1; i0 < NUM_CORNERS; ++i0, i1 = (i0 + 1) % NUM_CORNERS) {
            for (int vtx0 = 0, vtx1 = 1; vtx1 < baseVertexCount; ++vtx0, ++vtx1) {
                triangleData.addRawIndex(firstIndex + i0 * baseVertexCount + vtx0);
                triangleData.addRawIndex(firstIndex + i0 * baseVertexCount + vtx1 + secondIdxDelta);
                triangleData.addRawIndex(firstIndex + i1 * baseVertexCount + vtx1 + thirdIdxDelta);

                triangleData.addRawIndex(firstIndex + i1 * baseVertexCount + vtx1);
                triangleData.addRawIndex(firstIndex + i1 * baseVertexCount + vtx0 + secondIdxDelta);
                triangleData.addRawIndex(firstIndex + i0 * baseVertexCount + vtx0 + thirdIdxDelta);
            }
        }
    }

    QuarterTorusNode::QuarterTorusNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent) :
        GeneratedMeshNode(color, parent) {}

    std::string QuarterTorusNode::getDescription() {
        return "Quarter Torus";
    }

    mesh_identifier_t QuarterTorusNode::getMeshIdentifier() const {
        return constants::MESH_ID_QUARTER_TORUS;
    }

    void QuarterTorusNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {
        const ldr::ColorReference& color = ldr::color_repo::getInstanceDummyColor();
        auto& triangleData = mesh->getTriangleData(color);
        auto firstIndex = triangleData.getVertexCount();

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
                triangleData.addRawVertex(TriangleVertex{glm::vec4(baseVertices[j].position, 1.f) * rotation, glm::vec4(baseVertices[j].normal, 0.0f) * rotation});
            }
        }

        //todo windingOrder
        for (int i1 = 0, i2 = 1; i1 < DIVISIONS; ++i1, ++i2) {
            for (int j1 = 0, j2 = 1; j1 < DIVISIONS; ++j1, j2 = (j1 + 1) % DIVISIONS) {
                triangleData.addRawIndex(firstIndex + i1 * DIVISIONS + j1);
                triangleData.addRawIndex(firstIndex + i1 * DIVISIONS + j2);
                triangleData.addRawIndex(firstIndex + i2 * DIVISIONS + j2);

                triangleData.addRawIndex(firstIndex + i2 * DIVISIONS + j2);
                triangleData.addRawIndex(firstIndex + i2 * DIVISIONS + j1);
                triangleData.addRawIndex(firstIndex + i1 * DIVISIONS + j1);
            }
        }

        if (WITH_ENDS) {
            auto capAFirstIndex = triangleData.getVertexCount();
            for (int j = 0; j < DIVISIONS; ++j) {
                triangleData.addRawVertex(TriangleVertex{glm::vec4(baseVertices[j].position, 1.f), glm::vec3(0.0f, 0.0f, 1.0f)});
                if (j > 1) {
                    triangleData.addRawIndex(capAFirstIndex);
                    triangleData.addRawIndex(capAFirstIndex + j - 1);
                    triangleData.addRawIndex(capAFirstIndex + j);
                }
            }

            auto capBFirstIndex = triangleData.getVertexCount();
            auto endRotation = glm::rotate(0.5f * float(M_PI), glm::vec3(1.0f, 0.0f, 0.0f));
            for (int j = 0; j < DIVISIONS; ++j) {
                triangleData.addRawVertex(TriangleVertex{glm::vec4(baseVertices[j].position, 1.f) * endRotation, glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) * endRotation});
                if (j > 1) {
                    triangleData.addRawIndex(capBFirstIndex);
                    triangleData.addRawIndex(capBFirstIndex + j - 1);
                    triangleData.addRawIndex(capBFirstIndex + j);
                }
            }
        }
    }

    CubeNode::CubeNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent) :
        GeneratedMeshNode(color, parent) {}

    std::string CubeNode::getDescription() {
        return "Cube";
    }

    mesh_identifier_t CubeNode::getMeshIdentifier() const {
        return constants::MESH_ID_CUBE;
    }

    void CubeNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {
        const ldr::ColorReference& color = ldr::color_repo::getInstanceDummyColor();
        auto& triangleData = mesh->getTriangleData(color);
        if (triangleData.getVertexCount() > 0) {
            spdlog::warn("CubeNode::addToMesh verticesList not empty, existing vertices are removed");
        }
        if (windingInversed) {
            spdlog::warn("CubeNode::addToMesh with windingInversed currently not supported");
        }
        /* 2--------------3
          /|            / |      Z
         / |           /  |      |
        0-------------1   |      |
        |  7--------- | --6      *------- Y
        | /           |  /      /
        |/            | /      /
        4-------------5       X
         */
        constexpr float N = 0.5f;
        glm::vec4 pos[] = {
                {+N, -N, +N, 1.0f},//0
                {+N, +N, +N, 1.0f},//1
                {-N, -N, +N, 1.0f},//2
                {-N, +N, +N, 1.0f},//3
                {+N, -N, -N, 1.0f},//4
                {+N, +N, -N, 1.0f},//5
                {-N, +N, -N, 1.0f},//6
                {-N, -N, -N, 1.0f},//7
        };
        glm::vec3 normalXneg{-1, 0, 0};
        glm::vec3 normalXpos{+1, 0, 0};
        glm::vec3 normalYneg{0, -1, 0};
        glm::vec3 normalYpos{0, +1, 0};
        glm::vec3 normalZneg{0, 0, -1};
        glm::vec3 normalZpos{0, 0, +1};

        triangleData.addRawVertex({pos[0], normalXpos});
        triangleData.addRawVertex({pos[4], normalXpos});
        triangleData.addRawVertex({pos[5], normalXpos});
        triangleData.addRawVertex({pos[1], normalXpos});

        triangleData.addRawVertex({pos[3], normalXneg});
        triangleData.addRawVertex({pos[6], normalXneg});
        triangleData.addRawVertex({pos[7], normalXneg});
        triangleData.addRawVertex({pos[2], normalXneg});

        triangleData.addRawVertex({pos[1], normalYpos});
        triangleData.addRawVertex({pos[5], normalYpos});
        triangleData.addRawVertex({pos[6], normalYpos});
        triangleData.addRawVertex({pos[3], normalYpos});

        triangleData.addRawVertex({pos[2], normalYneg});
        triangleData.addRawVertex({pos[7], normalYneg});
        triangleData.addRawVertex({pos[4], normalYneg});
        triangleData.addRawVertex({pos[0], normalYneg});

        triangleData.addRawVertex({pos[0], normalZpos});
        triangleData.addRawVertex({pos[1], normalZpos});
        triangleData.addRawVertex({pos[3], normalZpos});
        triangleData.addRawVertex({pos[2], normalZpos});

        triangleData.addRawVertex({pos[5], normalZneg});
        triangleData.addRawVertex({pos[4], normalZneg});
        triangleData.addRawVertex({pos[7], normalZneg});
        triangleData.addRawVertex({pos[6], normalZneg});

        for (const auto idx: {
                     0 + 0, 0 + 1, 0 + 2, 0 + 2, 0 + 3, 0 + 0,      //X+
                     4 + 0, 4 + 1, 4 + 2, 4 + 2, 4 + 3, 4 + 0,      //X-
                     8 + 0, 8 + 1, 8 + 2, 8 + 2, 8 + 3, 8 + 0,      //Y+
                     12 + 0, 12 + 1, 12 + 2, 12 + 2, 12 + 3, 12 + 0,//Y-
                     16 + 0, 16 + 1, 16 + 2, 16 + 2, 16 + 3, 16 + 0,//Z+
                     20 + 0, 20 + 1, 20 + 2, 20 + 2, 20 + 3, 20 + 0,//Z-
             }) {
            triangleData.addRawIndex(idx);
        }
    }
}