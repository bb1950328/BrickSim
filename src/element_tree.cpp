// element_tree.cpp
// Created by bb1950328 on 02.10.20.
//

#include "element_tree.h"
#include "config.h"
#include "ldr_file_repository.h"
#include "ldr_colors.h"
#include "helpers/util.h"

const glm::mat4 &ElementTreeNode::getRelativeTransformation() const {
    return relativeTransformation;
}

void ElementTreeNode::setRelativeTransformation(const glm::mat4 &newValue) {
    relativeTransformation = newValue;
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

ElementTreeNode::ElementTreeNode(ElementTreeNode *parent) {
    this->parent = parent;
}

void ElementTreeLdrNode::addToMesh(Mesh *mesh, bool windingInversed) {
    LdrInstanceDummyColor *dummyColor = &LdrColorRepository::instDummyColor;
    for (auto element : ldrFile->elements) {
        switch (element->getType()) {
            case 0:
                break;
            case 1: {
                auto *sfElement = dynamic_cast<LdrSubfileReference *>(element);
                if (childrenWithOwnNode.find(sfElement)==childrenWithOwnNode.end()) {
                    mesh->addLdrSubfileReference(dummyColor, sfElement, glm::mat4(1.0f), windingInversed);
                }
            }
                break;
            case 2:
                mesh->addLdrLine(dummyColor, dynamic_cast<LdrLine &&>(*element), glm::mat4(1.0f));
                break;
            case 3:
                mesh->addLdrTriangle(dummyColor, dynamic_cast<LdrTriangle &&>(*element), glm::mat4(1.0f), windingInversed);
                break;
            case 4:
                mesh->addLdrQuadrilateral(dummyColor, dynamic_cast<LdrQuadrilateral &&>(*element), glm::mat4(1.0f), windingInversed);
                break;
            case 5:
                break;//todo implement optional lines
        }
    }
}

ElementTreeMpdSubfileNode *findMpdNodeAndAddSubfileNode(LdrFile *ldrFile, LdrColor* ldrColor, ElementTreeNode* actualNode) {
    if ((actualNode->getType()&ET_TYPE_MULTI_PART_DOCUMENT)>0) {
        //check if the subfileNode already exists
        for (const auto &child : actualNode->children) {
            if ((child->getType() & ET_TYPE_MPD_SUBFILE) > 0) {
                auto mpdSubfileChild = dynamic_cast<ElementTreeMpdSubfileNode *>(child);
                if (ldrFile == mpdSubfileChild->ldrFile) {
                    return mpdSubfileChild;
                }
            }
        }
        //the node doesn't exist, we have to create it
        auto *addedNode = new ElementTreeMpdSubfileNode(ldrFile, ldrColor);
        actualNode->children.push_back(addedNode);
        return addedNode;
    } else if (actualNode->parent == nullptr) {
        return nullptr;
    } else {
        return findMpdNodeAndAddSubfileNode(ldrFile, ldrColor, actualNode->parent);
    }
}

ElementTreeLdrNode::ElementTreeLdrNode(LdrFile *ldrFile, LdrColor *ldrColor, ElementTreeNode *parent) : ElementTreeMeshNode(ldrColor, parent), ldrFile(ldrFile){
    this->parent = parent;
    this->color = ldrColor;
    displayName = ldrFile->getDescription();
    for (const auto &element: ldrFile->elements) {
        if (element->getType() == 1) {
            auto *sfElement = dynamic_cast<LdrSubfileReference *>(element);
            auto *subFile = sfElement->getFile();
            if (subFile->metaInfo.type==MPD_SUBFILE) {
                auto* subFileNode = findMpdNodeAndAddSubfileNode(subFile, sfElement->color, this);
                auto* subFileInstanceNode = new ElementTreeMpdSubfileInstanceNode(subFileNode,
                                                                                  sfElement->color->code==16?ldrColor:sfElement->color,
                                                                                  this);
                children.push_back(subFileInstanceNode);
                childrenWithOwnNode.insert(sfElement);
            } else if (subFile->metaInfo.type==PART) {
                childrenWithOwnNode.insert(sfElement);
                LdrColor *color = sfElement->color->code==16?ldrColor:sfElement->color;
                auto *newNode = new ElementTreeLdrNode(subFile, color, this);
                newNode->setRelativeTransformation(sfElement->getTransformationMatrix());
                children.push_back(newNode);
            }
        }
    }
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
        default:
            return false;
    }
}

void ElementTree::loadLdrFile(const std::string &filename) {
    auto *newNode = new ElementTreeMpdNode(LdrFileRepository::get_file(filename), LdrColorRepository::getInstance()->get_color(1), &rootNode);
    rootNode.children.push_back(newNode);
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

ElementTreeRootNode::ElementTreeRootNode() : ElementTreeNode(nullptr) {
    displayName = "Root";
    absoluteTransformation = relativeTransformation;
    absoluteTransformationValid = true;
}

bool ElementTreeRootNode::isDisplayNameUserEditable() const {
    return false;
}

void *ElementTreeMpdSubfileInstanceNode::getMeshIdentifier() {
    return mpdSubfileNode->getMeshIdentifier();
}

void ElementTreeMpdSubfileInstanceNode::addToMesh(Mesh *mesh, bool windingInversed) {
    mpdSubfileNode->addToMesh(mesh, windingInversed);
}

std::string ElementTreeMpdSubfileInstanceNode::getDescription() {
    return mpdSubfileNode->getDescription();
}

ElementTreeNodeType ElementTreeMpdSubfileInstanceNode::getType() {
    return ET_TYPE_MPD_SUBFILE_INSTANCE;
}

bool ElementTreeMpdSubfileInstanceNode::isDisplayNameUserEditable() const {
    return false;
}

ElementTreeMpdSubfileInstanceNode::ElementTreeMpdSubfileInstanceNode(ElementTreeMpdSubfileNode *mpdSubfileNode,
                                                                     LdrColor* color,
                                                                     ElementTreeNode* parent)
                                                                     : mpdSubfileNode(mpdSubfileNode), ElementTreeMeshNode(color, parent) {

}

ElementTreeNodeType ElementTreeMpdNode::getType() {
    return ET_TYPE_MULTI_PART_DOCUMENT;
}

bool ElementTreeMpdNode::isDisplayNameUserEditable() const {
    return true;
}

ElementTreeMpdNode::ElementTreeMpdNode(LdrFile *ldrFile, LdrColor *ldrColor, ElementTreeNode *parent)
: ElementTreeLdrNode(ldrFile, ldrColor, parent) {

}

ElementTreeNodeType ElementTreeMpdSubfileNode::getType() {
    return ET_TYPE_MPD_SUBFILE;
}

bool ElementTreeMpdSubfileNode::isDisplayNameUserEditable() const {
    return true;
}

ElementTreeMpdSubfileNode::ElementTreeMpdSubfileNode(LdrFile *ldrFile, LdrColor *color) : ElementTreeLdrNode(ldrFile, color, parent) {

}

ElementTreeNodeType ElementTreePartNode::getType() {
    return ET_TYPE_PART;
}

bool ElementTreePartNode::isDisplayNameUserEditable() const {
    return false;
}

ElementTreePartNode::ElementTreePartNode(LdrFile *ldrFile, LdrColor *ldrColor, ElementTreeNode *parent)
: ElementTreeLdrNode(ldrFile, ldrColor, parent) {

}

ElementTreeMeshNode::ElementTreeMeshNode(LdrColor *color, ElementTreeNode *parent) : ElementTreeNode(parent) {
    this->color = color;
}
