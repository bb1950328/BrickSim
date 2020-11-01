// element_tree.h
// Created by bb1950328 on 02.10.20.
//

#ifndef BRICKSIM_ELEMENT_TREE_H
#define BRICKSIM_ELEMENT_TREE_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "ldr_files.h"
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
    util::RGBcolor getColorOfType(const NodeType &type);

    class Node {
    public:
        explicit Node(Node *parent);
        bool visible = true;
        Node *parent;
        std::string displayName;
        bool selected = false;
        NodeType type = TYPE_OTHER;

        [[nodiscard]] const glm::mat4 &getRelativeTransformation() const;
        void setRelativeTransformation(const glm::mat4 &newValue);
        const glm::mat4 &getAbsoluteTransformation();
        [[nodiscard]] virtual bool isTransformationUserEditable() const;

        NodeType getType();

        [[nodiscard]] virtual bool isDisplayNameUserEditable() const = 0;
        virtual std::string getDescription();

        [[nodiscard]] const std::vector<Node *> &getChildren() const;
        void addChild(Node *newChild);

    protected:
        std::vector<Node *> children;
        glm::mat4 relativeTransformation = glm::mat4(1.0f);
        glm::mat4 absoluteTransformation;
        bool absoluteTransformationValid = false;

        void invalidateAbsoluteTransformation();
    };

    class RootNode : public Node {
    public:
        RootNode();

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MeshNode : public Node {
    public:
        MeshNode(LdrColor *color, Node *parent);

        virtual void *getMeshIdentifier() = 0;
        virtual void addToMesh(Mesh *mesh, bool windingInversed) = 0;
        [[nodiscard]] virtual bool isColorUserEditable() const;
        [[nodiscard]] LdrColor *getDisplayColor() const;
        void setColor(LdrColor *newColor);
        LdrColor *getElementColor() const;
    private:
        LdrColor *color;
    };

    class LdrNode : public MeshNode {
    public:
        LdrNode(NodeType nodeType, LdrFile *ldrFile, LdrColor *ldrColor, Node *parent);

        void *getMeshIdentifier() override;
        void addToMesh(Mesh *mesh, bool windingInversed) override;
        std::string getDescription() override;
        LdrFile *ldrFile;
        std::set<LdrSubfileReference *> childrenWithOwnNode;

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MpdSubfileNode;

    class MpdSubfileInstanceNode : public MeshNode {
    public:
        MpdSubfileInstanceNode(MpdSubfileNode *mpdSubfileNode, LdrColor *color, Node *parent);

        MpdSubfileNode *mpdSubfileNode;
        void *getMeshIdentifier() override;
        void addToMesh(Mesh *mesh, bool windingInversed) override;
        std::string getDescription() override;
        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MpdNode : public LdrNode {
    public:
        MpdNode(LdrFile *ldrFile, LdrColor *ldrColor, Node *parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class MpdSubfileNode : public LdrNode {
    public:
        MpdSubfileNode(LdrFile *ldrFile, LdrColor *color, Node *parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
        [[nodiscard]] bool isTransformationUserEditable() const override;
        [[nodiscard]] bool isColorUserEditable() const override;
    };

    class PartNode : public LdrNode {
    public:
        PartNode(LdrFile *ldrFile, LdrColor *ldrColor, Node *parent);

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class ElementTree {
    public:
        RootNode rootNode;
        void loadLdrFile(const std::string &filename);
        void print();

    private:
        void printFromNode(int indent, Node *node);
    };
}


#endif //BRICKSIM_ELEMENT_TREE_H
