#pragma once

#include "../element_tree.h"
#include "../types.h"
#include "camera.h"
#include "mesh/mesh.h"

namespace bricksim::graphics::orientation_cube {
    enum class CubeSide {
        RIGHT,
        BOTTOM,
        BACK,
        LEFT,
        TOP,
        FRONT
    };

    namespace {
        class OrientationCubeSideMeshNode : public etree::MeshNode {
            const CubeSide side;

        public:
            mesh_identifier_t getMeshIdentifier() const override;

            bool isDisplayNameUserEditable() const override;

            OrientationCubeSideMeshNode(const std::shared_ptr<etree::Node>& parent, CubeSide side);

            void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;

            CubeSide getSide() const;
        };

        class OrientationCubeCamera : public Camera {
        private:
            float pitch = 1e9;
            float yaw = 1e9;
            glm::vec3 viewPos;
            glm::mat4 viewMatrix;
            glm::vec3 target{0.0f, 0.0f, 0.0f};

        public:
            void setPitchYaw(float newPitch, float newYaw);

            [[nodiscard]] const glm::mat4& getViewMatrix() const override;
            [[nodiscard]] const glm::vec3& getCameraPos() const override;
            [[nodiscard]] const glm::vec3& getTargetPos() const override;
        };
    }

    void initialize();
    unsigned int getImage();
    unsigned int getSelectionImage();
    std::optional<CubeSide> getSide(glm::usvec2 pos);
    uint16_t getSize();
    void cleanup();
}
