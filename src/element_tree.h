

#ifndef BRICKSIM_ELEMENT_TREE_H
#define BRICKSIM_ELEMENT_TREE_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "ldr_files/ldr_files.h"
#include "mesh.h"
#include "helpers/util.h"

namespace etree {
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

    const char *getDisplayNameOfType(const NodeType &type);
    color::RGB getColorOfType(const NodeType &type);

    class Node : public std::enable_shared_from_this<Node> {
    public:
        explicit Node(std::shared_ptr<Node> parent);
        Node(const Node&) = delete;
        bool visible = true;
        bool visibleInElementTree = true;
        std::weak_ptr<Node> parent;
        std::string displayName;
        bool selected = false;
        NodeType type = TYPE_OTHER;
        layer_t layer=0;

        [[nodiscard]] const glm::mat4 &getRelativeTransformation() const;
        void setRelativeTransformation(const glm::mat4 &newValue);
        const glm::mat4 &getAbsoluteTransformation() const;
        [[nodiscard]] virtual bool isTransformationUserEditable() const;

        NodeType getType() const;

        [[nodiscard]] virtual bool isDisplayNameUserEditable() const = 0;
        virtual std::string getDescription();

        [[nodiscard]] const std::vector<std::shared_ptr<Node>> &getChildren() const;
        void addChild(const std::shared_ptr<Node>& newChild);

        void deleteChild(const std::shared_ptr<Node>& childToDelete);
        virtual ~Node();
    protected:
        std::vector<std::shared_ptr<Node>> children;
        glm::mat4 relativeTransformation = glm::mat4(1.0f);
        mutable glm::mat4 absoluteTransformation;
        mutable bool absoluteTransformationValid = false;

        void invalidateAbsoluteTransformation();
    };

    class RootNode : public Node {
    public:
        RootNode();

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MeshNode : public Node {
    public:
        MeshNode(LdrColorReference color, std::shared_ptr<Node> parent);

        virtual void *getMeshIdentifier() const = 0;
        virtual void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) = 0;
        [[nodiscard]] virtual bool isColorUserEditable() const;
        [[nodiscard]] LdrColorReference getDisplayColor() const;
        void setColor(LdrColorReference newColor);
        LdrColorReference getElementColor() const;
    private:
        LdrColorReference color;
    };

    class LdrNode : public MeshNode {
    public:
        LdrNode(NodeType nodeType, const std::shared_ptr<LdrFile>& ldrFile, LdrColorReference ldrColor, const std::shared_ptr<Node>& parent);
        /**
         * This function is necessary because shared_from_this() doesn't work inside the constructor. so this function should be called immediately after creating
         * an object of type LdrNode. todo find a better solution for this
         */
        void createChildNodes();

        void *getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
        std::string getDescription() override;
        std::shared_ptr<LdrFile> ldrFile;
        std::set<std::shared_ptr<LdrSubfileReference>> childrenWithOwnNode;

        [[nodiscard]] bool isDisplayNameUserEditable() const override;

        /**
         * finds the subfileNode and creates a MpdSubfileInstanceNode as child of this
         */
        void addSubfileInstanceNode(const std::shared_ptr<LdrFile>& subFile, LdrColorReference instanceColor);
    private:
        bool childNodesCreated = false;
    };

    class MpdSubfileNode;

    class MpdSubfileInstanceNode : public MeshNode {
    public:
        MpdSubfileInstanceNode(const std::shared_ptr<MpdSubfileNode>& mpdSubfileNode, LdrColorReference color, std::shared_ptr<Node> parent);

        std::shared_ptr<MpdSubfileNode> mpdSubfileNode;
        void *getMeshIdentifier() const override;
        void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override;
        std::string getDescription() override;
        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MpdNode : public LdrNode {
    public:
        MpdNode(const std::shared_ptr<LdrFile>& ldrFile, LdrColorReference ldrColor, const std::shared_ptr<Node>& parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MpdSubfileNode : public LdrNode {
    public:
        MpdSubfileNode(const std::shared_ptr<LdrFile>& ldrFile, LdrColorReference color, const std::shared_ptr<Node>& parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
        [[nodiscard]] bool isTransformationUserEditable() const override;
        [[nodiscard]] bool isColorUserEditable() const override;
    };

    class PartNode : public LdrNode {
    public:
        PartNode(const std::shared_ptr<LdrFile>& ldrFile, LdrColorReference ldrColor, const std::shared_ptr<Node>& parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };
}


#endif //BRICKSIM_ELEMENT_TREE_H
