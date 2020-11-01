// element_tree.cpp
// Created by bb1950328 on 02.10.20.
//

#include "element_tree.h"
#include "config.h"
#include "ldr_file_repository.h"
#include "ldr_colors.h"

namespace etree {

    const glm::mat4 &Node::getRelativeTransformation() const {
        return relativeTransformation;
    }

    void Node::setRelativeTransformation(const glm::mat4 &newValue) {
        relativeTransformation = newValue;
        invalidateAbsoluteTransformation();
    }

    void Node::invalidateAbsoluteTransformation() {
        absoluteTransformationValid = false;
        for (const auto &child: children) {
            child->invalidateAbsoluteTransformation();
        }
    }

    const glm::mat4 &Node::getAbsoluteTransformation() {
        if (!absoluteTransformationValid) {
            absoluteTransformation = relativeTransformation * parent->getAbsoluteTransformation();
            absoluteTransformationValid = true;
        }
        return absoluteTransformation;
    }

    NodeType Node::getType() const {
        return type;
    }

    Node::Node(Node *parent) {
        this->parent = parent;
        type = TYPE_OTHER;
    }

    std::string Node::getDescription() {
        return displayName;
    }

    bool Node::isTransformationUserEditable() const {
        return true;
    }

    const std::vector<Node *> &Node::getChildren() const {
        return children;
    }

    void Node::addChild(Node *newChild) {
        children.push_back(newChild);
    }

    bool MeshNode::isColorUserEditable() const {
        return true;
    }

    void LdrNode::addToMesh(Mesh *mesh, bool windingInversed) {
        ldr_color_repo::LdrInstanceDummyColor *dummyColor = &ldr_color_repo::getInstanceDummyColor();
        for (auto element : ldrFile->elements) {
            switch (element->getType()) {
                case 0: break;
                case 1: {
                    auto *sfElement = dynamic_cast<LdrSubfileReference *>(element);
                    if (childrenWithOwnNode.find(sfElement) == childrenWithOwnNode.end()) {
                        mesh->addLdrSubfileReference(dummyColor, sfElement, glm::mat4(1.0f), windingInversed);
                    }
                }
                    break;
                case 2: mesh->addLdrLine(dummyColor, dynamic_cast<LdrLine &&>(*element), glm::mat4(1.0f));
                    break;
                case 3: mesh->addLdrTriangle(dummyColor, dynamic_cast<LdrTriangle &&>(*element), glm::mat4(1.0f), windingInversed);
                    break;
                case 4: mesh->addLdrQuadrilateral(dummyColor, dynamic_cast<LdrQuadrilateral &&>(*element), glm::mat4(1.0f), windingInversed);
                    break;
                case 5: mesh->addLdrOptionalLine(dummyColor, dynamic_cast<LdrOptionalLine &&>(*element), glm::mat4(1.0f));
                    break;
            }
        }
    }

    MpdSubfileNode *findMpdNodeAndAddSubfileNode(LdrFile *ldrFile, LdrColor *ldrColor, Node *actualNode) {
        if (actualNode->getType() == TYPE_MULTI_PART_DOCUMENT) {
            //check if the subfileNode already exists
            for (const auto &child : actualNode->getChildren()) {
                if (child->getType() == TYPE_MPD_SUBFILE) {
                    auto mpdSubfileChild = dynamic_cast<MpdSubfileNode *>(child);
                    if (ldrFile == mpdSubfileChild->ldrFile) {
                        return mpdSubfileChild;
                    }
                }
            }
            //the node doesn't exist, we have to create it
            auto *addedNode = new MpdSubfileNode(ldrFile, ldrColor, actualNode);
            actualNode->addChild(addedNode);
            return addedNode;
        } else if (actualNode->parent == nullptr) {
            return nullptr;
        } else {
            return findMpdNodeAndAddSubfileNode(ldrFile, ldrColor, actualNode->parent);
        }
    }

    LdrNode::LdrNode(NodeType nodeType, LdrFile *ldrFile, LdrColor *ldrColor, Node *parent) : MeshNode(ldrColor, parent), ldrFile(ldrFile) {
        type = nodeType;
        this->parent = parent;
        this->setColor(ldrColor);
        displayName = ldrFile->getDescription();
        for (const auto &element: ldrFile->elements) {
            if (element->getType() == 1) {
                auto *sfElement = dynamic_cast<LdrSubfileReference *>(element);
                auto *subFile = sfElement->getFile();
                Node *newNode = nullptr;
                if (subFile->metaInfo.type == MPD_SUBFILE) {
                    auto *subFileNode = findMpdNodeAndAddSubfileNode(subFile, sfElement->color, this);
                    newNode = new MpdSubfileInstanceNode(subFileNode, sfElement->color, this);
                    childrenWithOwnNode.insert(sfElement);
                } else if (subFile->metaInfo.type == PART) {
                    childrenWithOwnNode.insert(sfElement);
                    newNode = new PartNode(subFile, sfElement->color, this);
                }
                if (nullptr != newNode) {
                    children.push_back(newNode);
                    newNode->setRelativeTransformation(sfElement->getTransformationMatrix());
                }
            }
        }
    }

    void *LdrNode::getMeshIdentifier() {
        return reinterpret_cast<void *>(ldrFile);
    }

    std::string LdrNode::getDescription() {
        return ldrFile->getDescription();
    }

    bool LdrNode::isDisplayNameUserEditable() const {
        switch (ldrFile->metaInfo.type) {
            case MODEL:
            case MPD_SUBFILE:return true;
            default:return false;
        }
    }

    void LdrNode::addSubfileInstanceNode(LdrFile *subFile, LdrColor* instanceColor) {
        MpdSubfileNode *subfileNode = findMpdNodeAndAddSubfileNode(subFile, ldr_color_repo::get_color(1), this);
        this->children.push_back(new MpdSubfileInstanceNode(subfileNode, instanceColor, this));
    }

    Node* ElementTree::loadLdrFile(const std::string &filename) {
        auto *newNode = new MpdNode(ldr_file_repo::get_file(filename), ldr_color_repo::get_color(2), &rootNode);
        rootNode.addChild(newNode);
        return newNode;
    }

    void ElementTree::print() {
        printFromNode(0, &rootNode);
    }

    void ElementTree::printFromNode(int indent, Node *node) {
        for (int i = 0; i < indent; ++i) {
            std::cout << ' ';
        }
        std::cout << node->displayName << std::endl;
        for (const auto &child: node->getChildren()) {
            printFromNode(indent + 2, child);
        }
    }

    RootNode::RootNode() : Node(nullptr) {
        type = TYPE_ROOT;
        displayName = "Root";
        absoluteTransformation = relativeTransformation;
        absoluteTransformationValid = true;
    }

    bool RootNode::isDisplayNameUserEditable() const {
        return false;
    }

    void *MpdSubfileInstanceNode::getMeshIdentifier() {
        return mpdSubfileNode->getMeshIdentifier();
    }

    void MpdSubfileInstanceNode::addToMesh(Mesh *mesh, bool windingInversed) {
        mpdSubfileNode->addToMesh(mesh, windingInversed);
    }

    std::string MpdSubfileInstanceNode::getDescription() {
        return mpdSubfileNode->getDescription();
    }

    bool MpdSubfileInstanceNode::isDisplayNameUserEditable() const {
        return false;
    }

    MpdSubfileInstanceNode::MpdSubfileInstanceNode(MpdSubfileNode *mpdSubfileNode, LdrColor *color, Node *parent) : mpdSubfileNode(mpdSubfileNode), MeshNode(color, parent) {
        type = TYPE_MPD_SUBFILE_INSTANCE;
        this->displayName = mpdSubfileNode->displayName;
    }

    bool MpdSubfileNode::isTransformationUserEditable() const {
        return false;
    }

    bool MpdSubfileNode::isColorUserEditable() const {
        return false;
    }

    bool MpdNode::isDisplayNameUserEditable() const {
        return true;
    }

    MpdNode::MpdNode(LdrFile *ldrFile, LdrColor *ldrColor, Node *parent) : LdrNode(TYPE_MULTI_PART_DOCUMENT, ldrFile, ldrColor, parent) {
    }

    bool MpdSubfileNode::isDisplayNameUserEditable() const {
        return true;
    }

    MpdSubfileNode::MpdSubfileNode(LdrFile *ldrFile, LdrColor *color, Node *parent) : LdrNode(TYPE_MPD_SUBFILE, ldrFile, color, parent) {
        visible = false;
    }

    bool PartNode::isDisplayNameUserEditable() const {
        return false;
    }

    PartNode::PartNode(LdrFile *ldrFile, LdrColor *ldrColor, Node *parent) : LdrNode(TYPE_PART, ldrFile, ldrColor, parent) {
    }

    MeshNode::MeshNode(LdrColor *color, Node *parent) : Node(parent) {
        this->color = color;
        type = TYPE_MESH;
    }

    LdrColor *MeshNode::getDisplayColor() const {
        if (color->code == LdrColor::MAIN_COLOR_CODE && parent != nullptr && (parent->getType() & TYPE_MESH) > 0) {
            return dynamic_cast<MeshNode *>(parent)->getDisplayColor();
        } else {
            return color;
        }
    }

    void MeshNode::setColor(LdrColor *newColor) {
        MeshNode::color = newColor;
    }

    LdrColor *MeshNode::getElementColor() const {
        return color;
    }

    const char *getDisplayNameOfType(const NodeType &type) {
        switch (type) {
            case TYPE_ROOT: return "Root";
            case TYPE_MESH: return "Mesh";
            case TYPE_MPD_SUBFILE_INSTANCE: return "MPD subfile Instance";
            case TYPE_LDRFILE: return "LDraw file";
            case TYPE_MPD_SUBFILE: return "MPD subfile";
            case TYPE_MULTI_PART_DOCUMENT: return "MPD file";
            case TYPE_PART: return "Part";
            case TYPE_OTHER:
            default: return "Other";
        }
    }

    util::RGBcolor getColorOfType(const NodeType &type) {
        switch (type) {
            case TYPE_MPD_SUBFILE_INSTANCE: return config::get_color(config::COLOR_MPD_SUBFILE_INSTANCE);
            case TYPE_MPD_SUBFILE: return config::get_color(config::COLOR_MPD_SUBFILE);
            case TYPE_MULTI_PART_DOCUMENT: return config::get_color(config::COLOR_MULTI_PART_DOCUMENT);
            case TYPE_PART: return config::get_color(config::COLOR_OFFICAL_PART);//todo unoffical part
            case TYPE_OTHER:
            case TYPE_ROOT:
            case TYPE_MESH:
            case TYPE_LDRFILE:
            default: return util::RGBcolor(255, 255, 255);
        }
    }
}