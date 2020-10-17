// element_tree.cpp
// Created by bb1950328 on 02.10.20.
//

#include "element_tree.h"
#include "config.h"
#include "ldr_file_repository.h"
#include "ldr_colors.h"
#include "util.h"

const glm::mat4 &ElementTreeNode::getRelativeTransformation() const {
    return relativeTransformation;
}

void ElementTreeNode::setRelativeTransformation(const glm::mat4 &newValue) {
    ElementTreeNode::relativeTransformation = newValue;
    util::cout_mat4(newValue);
    invalidateAbsoluteTransformation();
}

void ElementTreeNode::invalidateAbsoluteTransformation() {
    absoluteTransformationValid = false;
    for (const auto &child: children) {
        child->invalidateAbsoluteTransformation();
    }
}

const glm::mat4 &ElementTreeNode::getAbsoluteTransformation() {
    if (!absoluteTransformationValid) {
        absoluteTransformation =  relativeTransformation * parent->getAbsoluteTransformation();
        absoluteTransformationValid = true;
    }
    return absoluteTransformation;
}

ElementTreeNodeType ElementTreeNode::getType() {
    return ET_TYPE_OTHER;
}

void ElementTreeLdrNode::addToMesh(Mesh *mesh) {
    LdrInstanceDummyColor *dummyColor = &LdrColorRepository::instDummyColor;
    for (auto element : ldrFile->elements) {
        switch (element->getType()) {
            case 0:
                break;
            case 1: {
                auto *sfElement = dynamic_cast<LdrSubfileReference *>(element);
                if (childrenWithOwnNode.find(sfElement)==childrenWithOwnNode.end()) {
                    mesh->addLdrSubfileReference(dummyColor, sfElement, glm::mat4(1.0f));
                }
            }
                break;
            case 2:
                mesh->addLdrLine(dummyColor, dynamic_cast<LdrLine &&>(*element), glm::mat4(1.0f));
                break;
            case 3:
                mesh->addLdrTriangle(dummyColor, dynamic_cast<LdrTriangle &&>(*element), glm::mat4(1.0f));
                break;
            case 4:
                mesh->addLdrQuadrilateral(dummyColor, dynamic_cast<LdrQuadrilateral &&>(*element), glm::mat4(1.0f));
                break;
            case 5:
                break;//todo implement optional lines
        }
    }
}

ElementTreeLdrNode::ElementTreeLdrNode(LdrFile *ldrFile, LdrColor *ldrColor) : ldrFile(ldrFile) {
    this->color = ldrColor;
    displayName = ldrFile->getDescription();
    for (const auto &element: ldrFile->elements) {
        if (element->getType() == 1) {
            auto *sfElement = dynamic_cast<LdrSubfileReference *>(element);
            auto *subFile = sfElement->getFile();
            if (subFile->isComplexEnoughForOwnMesh()) {
                childrenWithOwnNode.insert(sfElement);
                LdrColor *color = sfElement->color->code==16?ldrColor:sfElement->color;
                auto *newNode = new ElementTreeLdrNode(subFile, color);
                newNode->setRelativeTransformation(sfElement->getTransformationMatrix());
                newNode->parent = this;
                children.push_back(newNode);
            }
        }
    }
}

ElementTreeNodeType ElementTreeLdrNode::getType() {
    return ET_TYPE_LDRFILE;
}

void* ElementTreeLdrNode::getMeshIdentifier() {
    return reinterpret_cast<void*>(ldrFile);
}

std::string ElementTreeLdrNode::getDescription() {
    return ldrFile->getDescription();
}

bool ElementTreeLdrNode::isDisplayNameUserEditable() const {
    switch (ldrFile->metaInfo.type) {
        case MODEL:
        case MPD_SUBFILE:
            return true;
        case PART:
        case SUBPART:
        case PRIMITIVE:
            return false;
    }
}

void ElementTree::loadLdrFile(const std::string &filename) {
    auto *newNode = new ElementTreeLdrNode(LdrFileRepository::get_file(filename), LdrColorRepository::getInstance()->get_color(1));
    rootNode.children.push_back(newNode);
    newNode->parent = &rootNode;
}

void ElementTree::print() {
    printFromNode(0, &rootNode);
}

void ElementTree::printFromNode(int indent, ElementTreeNode *node) {
    for (int i = 0; i < indent; ++i) {
        std::cout << ' ';
    }
    std::cout << node->displayName << std::endl;
    for (const auto &child: node->children) {
        printFromNode(indent+2, child);
    }
}

ElementTreeNodeType ElementTreeRootNode::getType() {
    return ET_TYPE_ROOT;
}

ElementTreeRootNode::ElementTreeRootNode() {
    displayName = "Root";
    absoluteTransformation = relativeTransformation;
    absoluteTransformationValid = true;
}

bool ElementTreeRootNode::isDisplayNameUserEditable() const {
    return false;
}
