// element_tree.cpp
// Created by bb1950328 on 02.10.20.
//

#include "element_tree.h"
#include "config.h"
#include "ldr_file_repository.h"
#include "ldr_colors.h"

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
    return type;
}

ElementTreeNode::ElementTreeNode(ElementTreeNode *parent) {
    this->parent = parent;
    type=ET_TYPE_OTHER;
}

std::string ElementTreeNode::getDescription() {
    return displayName;
}

bool ElementTreeNode::isTransformationUserEditable() const {
    return true;
}

const std::vector<ElementTreeNode *> &ElementTreeNode::getChildren() const {
    return children;
}

void ElementTreeNode::addChild(ElementTreeNode *newChild) {
    children.push_back(newChild);
}

bool ElementTreeMeshNode::isColorUserEditable() const {
    return true;
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
    if (actualNode->getType()==ET_TYPE_MULTI_PART_DOCUMENT) {
        //check if the subfileNode already exists
        for (const auto &child : actualNode->getChildren()) {
            if (child->getType() == ET_TYPE_MPD_SUBFILE) {
                auto mpdSubfileChild = dynamic_cast<ElementTreeMpdSubfileNode *>(child);
                if (ldrFile == mpdSubfileChild->ldrFile) {
                    return mpdSubfileChild;
                }
            }
        }
        //the node doesn't exist, we have to create it
        auto *addedNode = new ElementTreeMpdSubfileNode(ldrFile, ldrColor, actualNode);
        actualNode->addChild(addedNode);
        return addedNode;
    } else if (actualNode->parent == nullptr) {
        return nullptr;
    } else {
        return findMpdNodeAndAddSubfileNode(ldrFile, ldrColor, actualNode->parent);
    }
}

ElementTreeLdrNode::ElementTreeLdrNode(ElementTreeNodeType nodeType, LdrFile *ldrFile, LdrColor *ldrColor, ElementTreeNode *parent) : ElementTreeMeshNode(ldrColor, parent), ldrFile(ldrFile){
    type=nodeType;
    this->parent = parent;
    this->color = ldrColor;
    displayName = ldrFile->getDescription();
    for (const auto &element: ldrFile->elements) {
        if (element->getType() == 1) {
            auto *sfElement = dynamic_cast<LdrSubfileReference *>(element);
            auto *subFile = sfElement->getFile();
            ElementTreeNode *newNode = nullptr;
            if (subFile->metaInfo.type==MPD_SUBFILE) {
                auto* subFileNode = findMpdNodeAndAddSubfileNode(subFile, sfElement->color, this);
                newNode = new ElementTreeMpdSubfileInstanceNode(subFileNode,
                                                                sfElement->color->code==16?ldrColor:sfElement->color,
                                                                this);
                childrenWithOwnNode.insert(sfElement);
            } else if (subFile->metaInfo.type==PART) {
                childrenWithOwnNode.insert(sfElement);
                LdrColor *color = sfElement->color->code==16?ldrColor:sfElement->color;//todo make that changes from parent are passed down
                newNode = new ElementTreePartNode(subFile, color, this);
            }
            if (nullptr!=newNode) {
                children.push_back(newNode);
                newNode->setRelativeTransformation(sfElement->getTransformationMatrix());
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
    rootNode.addChild(newNode);
}

void ElementTree::print() {
    printFromNode(0, &rootNode);
}

void ElementTree::printFromNode(int indent, ElementTreeNode *node) {
    for (int i = 0; i < indent; ++i) {
        std::cout << ' ';
    }
    std::cout << node->displayName << std::endl;
    for (const auto &child: node->getChildren()) {
        printFromNode(indent+2, child);
    }
}

ElementTreeRootNode::ElementTreeRootNode() : ElementTreeNode(nullptr) {
    type = ET_TYPE_ROOT;
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

bool ElementTreeMpdSubfileInstanceNode::isDisplayNameUserEditable() const {
    return false;
}

ElementTreeMpdSubfileInstanceNode::ElementTreeMpdSubfileInstanceNode(ElementTreeMpdSubfileNode *mpdSubfileNode,
                                                                     LdrColor* color,
                                                                     ElementTreeNode* parent)
                                                                     : mpdSubfileNode(mpdSubfileNode), ElementTreeMeshNode(color, parent) {
    type=ET_TYPE_MPD_SUBFILE_INSTANCE;
    this->displayName=mpdSubfileNode->displayName;
}

bool ElementTreeMpdSubfileNode::isTransformationUserEditable() const {
    return false;
}

bool ElementTreeMpdSubfileNode::isColorUserEditable() const {
    return false;
}

bool ElementTreeMpdNode::isDisplayNameUserEditable() const {
    return true;
}

ElementTreeMpdNode::ElementTreeMpdNode(LdrFile *ldrFile, LdrColor *ldrColor, ElementTreeNode *parent)
: ElementTreeLdrNode(ET_TYPE_MULTI_PART_DOCUMENT, ldrFile, ldrColor, parent) {
}

bool ElementTreeMpdSubfileNode::isDisplayNameUserEditable() const {
    return true;
}

ElementTreeMpdSubfileNode::ElementTreeMpdSubfileNode(LdrFile *ldrFile, LdrColor *color, ElementTreeNode* parent) : ElementTreeLdrNode(ET_TYPE_MPD_SUBFILE,  ldrFile, color, parent) {
    visible= false;
}

bool ElementTreePartNode::isDisplayNameUserEditable() const {
    return false;
}

ElementTreePartNode::ElementTreePartNode(LdrFile *ldrFile, LdrColor *ldrColor, ElementTreeNode *parent)
: ElementTreeLdrNode(ET_TYPE_PART,  ldrFile, ldrColor, parent) {
}

ElementTreeMeshNode::ElementTreeMeshNode(LdrColor *color, ElementTreeNode *parent) : ElementTreeNode(parent) {
    this->color = color;
    type=ET_TYPE_MESH;
}


const char *getDisplayNameOfType(const ElementTreeNodeType &type) {
    switch (type) {
        case ET_TYPE_ROOT: return "Root";
        case ET_TYPE_MESH: return "Mesh";
        case ET_TYPE_MPD_SUBFILE_INSTANCE: return "MPD subfile Instance";
        case ET_TYPE_LDRFILE: return "LDraw file";
        case ET_TYPE_MPD_SUBFILE: return "MPD subfile";
        case ET_TYPE_MULTI_PART_DOCUMENT: return "MPD file";
        case ET_TYPE_PART: return "Part";
        case ET_TYPE_OTHER:
        default: return "Other";
    }
}

util::RGBcolor getColorOfType(const ElementTreeNodeType &type) {
    switch (type) {
        case ET_TYPE_MPD_SUBFILE_INSTANCE:
            return config::get_color(config::COLOR_MPD_SUBFILE_INSTANCE);
        case ET_TYPE_MPD_SUBFILE:
            return config::get_color(config::COLOR_MPD_SUBFILE);
        case ET_TYPE_MULTI_PART_DOCUMENT:
            return config::get_color(config::COLOR_MULTI_PART_DOCUMENT);
        case ET_TYPE_PART:
            return config::get_color(config::COLOR_OFFICAL_PART);//todo unoffical part
        case ET_TYPE_OTHER:
        case ET_TYPE_ROOT:
        case ET_TYPE_MESH:
        case ET_TYPE_LDRFILE:
        default:
            return util::RGBcolor(255, 255, 255);
    }
}
