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
    ET_TYPE_OTHER,
    ET_TYPE_LDRFILE,
    ET_TYPE_ROOT
};

class ElementTreeNode {
public:
    bool visible = true;
    ElementTreeNode *parent;
    std::vector<ElementTreeNode *> children;
    std::string displayName;
    [[nodiscard]] const glm::mat4 &getRelativeTransformation() const;
    void setRelativeTransformation(const glm::mat4 &newValue);

    const glm::mat4 &getAbsoluteTransformation();

    virtual ElementTreeNodeType getType();

    virtual void addToMesh(Mesh *mesh) = 0;

protected:
    glm::mat4 relativeTransformation = glm::mat4(1.0f);
    glm::mat4 absoluteTransformation;
    bool absoluteTransformationValid = false;
};

class ElementTreeRootNode : public ElementTreeNode {
public:
    ElementTreeRootNode();
    ElementTreeNodeType getType() override;
    void addToMesh(Mesh *mesh) override;
};

class ElementTreeLdrNode : public ElementTreeNode {
public:
    void addToMesh(Mesh *mesh) override;
    LdrFile *ldrFile;
    LdrColor *ldrColor;
    std::set<LdrSubfileReference *> childrenWithOwnNode;
    /**
     * @param subfileReference
     * @return true if the SubfileReference should be "inlined" in the mesh
     *         false if it has its own mesh
     */
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
