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

enum ElementTreeNodeType {
    ET_TYPE_OTHER = 0,
    ET_TYPE_MESH = 1u << 0u,
    ET_TYPE_LDRFILE = (1u << 1u)|ET_TYPE_MESH,
    ET_TYPE_ROOT = 1u << 2u
};

class ElementTreeNode {
public:
    bool visible = true;
    ElementTreeNode *parent;
    std::vector<ElementTreeNode *> children;
    std::string displayName;
    bool selected = false;
    [[nodiscard]] const glm::mat4 &getRelativeTransformation() const;
    void setRelativeTransformation(const glm::mat4 &newValue);

    const glm::mat4 &getAbsoluteTransformation();

    virtual ElementTreeNodeType getType();

protected:
    glm::mat4 relativeTransformation = glm::mat4(1.0f);
    glm::mat4 absoluteTransformation;
    bool absoluteTransformationValid = false;

    void invalidateAbsoluteTransformation();
};

class ElementTreeRootNode : public ElementTreeNode {
public:
    ElementTreeRootNode();
    ElementTreeNodeType getType() override;
};

class ElementTreeMeshNode : public ElementTreeNode {
public:
    virtual void* getMeshIdentifier() = 0;
    virtual void addToMesh(Mesh *mesh) = 0;
    virtual std::string getDescription() = 0;
    LdrColor *color;
    std::optional<size_t> instanceIndex;
};

class ElementTreeLdrNode : public ElementTreeMeshNode {
public:
    void* getMeshIdentifier() override;
    void addToMesh(Mesh *mesh) override;
    std::string getDescription() override;
    LdrFile *ldrFile;
    std::set<LdrSubfileReference *> childrenWithOwnNode;
    ElementTreeNodeType getType() override;
    ElementTreeLdrNode(LdrFile *ldrFile, LdrColor *ldrColor);
};

class ElementTree {
public:
    ElementTreeRootNode rootNode;
    void loadLdrFile(const std::string &filename);
    void print();

private:
    void printFromNode(int indent, ElementTreeNode *node);
};

#endif //BRICKSIM_ELEMENT_TREE_H
