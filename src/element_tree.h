#pragma once

#include "binary_file.h"
#include "constant_data/constants.h"
#include "graphics/mesh/mesh.h"
#include "helpers/color.h"
#include "ldr/colors.h"
#include <memory>

namespace bricksim::etree {
    enum NodeType {
        TYPE_OTHER = 0,
        TYPE_ROOT = 1u << 0u,
        TYPE_MESH = 1u << 1u,
        TYPE_MPD_SUBFILE_INSTANCE = (1u << 2u) | TYPE_MESH,
        TYPE_LDRFILE = (1u << 3u) | TYPE_MESH,
        TYPE_MPD_SUBFILE = (1u << 4u) | TYPE_LDRFILE,
        TYPE_MULTI_PART_DOCUMENT = (1u << 5u) | TYPE_LDRFILE,
        TYPE_PART = (1u << 6u) | TYPE_LDRFILE,
    };

    const char* getDisplayNameOfType(const NodeType& type);
    color::RGB getColorOfType(const NodeType& type);

    class Node : public std::enable_shared_from_this<Node> {
    public:
        explicit Node(const std::shared_ptr<Node>& parent);
        Node(const Node&) = delete;
        bool visible = true;
        bool visibleInElementTree = true;
        std::weak_ptr<Node> parent;
        std::string displayName;
        bool selected = false;
        NodeType type = TYPE_OTHER;
        layer_t layer = constants::DEFAULT_LAYER;//todo think about inheritance of this attribute

        [[nodiscard]] const glm::mat4& getRelativeTransformation() const;
        void setRelativeTransformation(const glm::mat4& newValue);
        const glm::mat4& getAbsoluteTransformation() const;
        [[nodiscard]] virtual bool isTransformationUserEditable() const;

        NodeType getType() const;

        [[nodiscard]] virtual bool isDisplayNameUserEditable() const = 0;
        virtual std::string getDescription();

        [[nodiscard]] const std::vector<std::shared_ptr<Node>>& getChildren() const;
        void addChild(const std::shared_ptr<Node>& newChild);
        bool isChildOf(const std::shared_ptr<Node>& possibleParent) const;
        void removeChild(const std::shared_ptr<Node>& childToDelete);

        uint64_t getVersion() const;
        void incrementVersion();

        std::shared_ptr<Node> getRoot();

        virtual ~Node();

    protected:
        std::vector<std::shared_ptr<Node>> children;
        glm::mat4 relativeTransformation = glm::mat4(1.0f);
        mutable glm::mat4 absoluteTransformation;
        mutable bool absoluteTransformationValid = false;
        uint64_t version = 0;

        void invalidateAbsoluteTransformation();
    };

    class RootNode : public Node {
    public:
        RootNode();

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MeshNode : public Node {
    public:
        MeshNode(ldr::ColorReference color, const std::shared_ptr<Node>& parent);

        virtual mesh_identifier_t getMeshIdentifier() const = 0;
        virtual void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) = 0;
        [[nodiscard]] virtual bool isColorUserEditable() const;
        [[nodiscard]] ldr::ColorReference getDisplayColor() const;
        void setColor(ldr::ColorReference newColor);
        ldr::ColorReference getElementColor() const;

    private:
        ldr::ColorReference color;
    };

    class LdrNode : public MeshNode {
    public:
        LdrNode(NodeType nodeType, const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent);
        /**
         * This function is necessary because shared_from_this() doesn't work inside the constructor. so this function should be called immediately after creating
         * an object of type LdrNode. todo find a better solution for this
         */
        void createChildNodes();

        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) override;
        std::string getDescription() override;
        std::shared_ptr<ldr::File> ldrFile;
        uoset_t<std::shared_ptr<ldr::SubfileReference>> childrenWithOwnNode;

        [[nodiscard]] bool isDisplayNameUserEditable() const override;

        /**
         * finds the subfileNode and creates a MpdSubfileInstanceNode as child of this
         */
        void addSubfileInstanceNode(const std::shared_ptr<ldr::File>& subFile, ldr::ColorReference instanceColor);
        void writeChangesToLdrFile();

    private:
        bool childNodesCreated = false;
        struct ChildNodeSaveInfo {
            uint64_t lastSaveToLdrFileVersion = 0;
            std::shared_ptr<ldr::FileElement> ldrElement;
        };
        uomap_t<std::shared_ptr<Node>, ChildNodeSaveInfo> subfileRefChildNodeSaveInfos;
        uint64_t lastSaveToLdrFileVersion = 0;
    };

    class MpdSubfileNode;

    class MpdSubfileInstanceNode : public MeshNode {
    public:
        MpdSubfileInstanceNode(const std::shared_ptr<MpdSubfileNode>& mpdSubfileNode, ldr::ColorReference color, const std::shared_ptr<Node>& parent);

        std::shared_ptr<MpdSubfileNode> mpdSubfileNode;
        mesh_identifier_t getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) override;
        std::string getDescription() override;
        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MpdNode : public LdrNode {
    public:
        MpdNode(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MpdSubfileNode : public LdrNode {
    public:
        MpdSubfileNode(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference color, const std::shared_ptr<Node>& parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
        [[nodiscard]] bool isTransformationUserEditable() const override;
        [[nodiscard]] bool isColorUserEditable() const override;
    };

    class PartNode : public LdrNode {
    public:
        PartNode(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class TexmapNode : public MeshNode {
    public:
        TexmapNode(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, const std::shared_ptr<Node>& parent);

        mesh_identifier_t getMeshIdentifier() const override;

        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) override;

    private:
        ldr::TexmapStartCommand::ProjectionMethod projectionMethod;
        glm::vec3 p1, p2, p3;
        std::string textureFilename;
        std::shared_ptr<BinaryFile> textureFile;
        float a, b;
    };

    std::shared_ptr<Node> getFirstSelectedNode(std::shared_ptr<Node> rootNode);
}
