#ifndef BRICKSIM_GENERATED_MESH_H
#define BRICKSIM_GENERATED_MESH_H

#include "element_tree.h"

namespace generated_mesh {

    class GeneratedMeshNode : public etree::MeshNode {
    public:
        GeneratedMeshNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent);
        bool isDisplayNameUserEditable() const override;
        bool isTransformationUserEditable() const override;
        bool isColorUserEditable() const override;
    };

    class UVSphereNode : public GeneratedMeshNode {
    public:
        static constexpr float RADIUS = 0.5f;
        static constexpr uint16_t DIVISIONS = 12;
        UVSphereNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
    };

    class ArrowNode : public GeneratedMeshNode {
    public:
        ArrowNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent);
        std::string getDescription() override;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
    };
}

#endif //BRICKSIM_GENERATED_MESH_H
