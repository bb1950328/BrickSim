// element_tree.h
// Created by bab21 on 02.10.20.
//

#ifndef BRICKSIM_ELEMENT_TREE_H
#define BRICKSIM_ELEMENT_TREE_H
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "ldr_objects.h"
#include "mesh.h"

enum ElementTreeNodeType {
    ET_TYPE_OTHER,
    ET_TYPE_LDRFILE
};

class ElementTreeNode {
public:
    ElementTreeNode *parent;
    std::vector<ElementTreeNode *> children;
    std::string displayName;
private:
    glm::mat4 relativeTransformation = glm::mat4(1.0f);
public:
    [[nodiscard]] const glm::mat4 &getRelativeTransformation() const;

    void setRelativeTransformation(const glm::mat4 &newValue);

    const glm::mat4 &getAbsoluteTransformation();

    virtual ElementTreeNodeType getType();

    virtual void addToMesh(Mesh *mesh) = 0;
private:
    glm::mat4 absoluteTransformation;
    bool absoluteTransformationValid = false;
};

class ElementTreeLdrNode : public ElementTreeNode {
public:
    void addToMesh(Mesh *mesh) override;
    LdrFile *ldrFile;
    LdrColor *ldrColor;
    /**
     * @param subfileReference
     * @return true if the SubfileReference should be "inlined" in the mesh
     *         false if it has its own mesh
     */
    bool isAddSubfileReferenceToMesh(LdrSubfileReference *subfileReference);
    ElementTreeNodeType getType() override;
    ElementTreeLdrNode(LdrFile *ldrFile, LdrColor *ldrColor);
};

#endif //BRICKSIM_ELEMENT_TREE_H
