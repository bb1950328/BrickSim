#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
#include "element_tree.h"
#include "config.h"
#include <palanteer.h>

namespace bricksim::etree {

    const glm::mat4& Node::getRelativeTransformation() const {
        return relativeTransformation;
    }

    void Node::setRelativeTransformation(const glm::mat4& newValue) {
        relativeTransformation = newValue;
        invalidateAbsoluteTransformation();
    }

    void Node::invalidateAbsoluteTransformation() {
        absoluteTransformationValid = false;
        for (const auto& child: children) {
            child->invalidateAbsoluteTransformation();
        }
    }

    const glm::mat4& Node::getAbsoluteTransformation() const {
        if (!absoluteTransformationValid) {
            absoluteTransformation = relativeTransformation * parent.lock()->getAbsoluteTransformation();
            absoluteTransformationValid = true;
        }
        return absoluteTransformation;
    }

    NodeType Node::getType() const {
        return type;
    }

    Node::Node(const std::shared_ptr<Node>& parent) :
        parent(parent) {
        type = TYPE_OTHER;
    }

    std::string Node::getDescription() {
        return displayName;
    }

    bool Node::isTransformationUserEditable() const {
        return true;
    }

    const std::vector<std::shared_ptr<Node>>& Node::getChildren() const {
        return children;
    }

    void Node::addChild(const std::shared_ptr<Node>& newChild) {
        children.push_back(newChild);
    }

    void Node::removeChild(const std::shared_ptr<Node>& childToDelete) {
        auto it = children.begin();
        while (*it != childToDelete && it != children.end()) {
            ++it;
        }
        if (it != children.end()) {
            children.erase(it);
        }
    }

    bool Node::isChildOf(const std::shared_ptr<Node>& possibleParent) const {
        auto parentLocked = parent.lock();
        return parentLocked == possibleParent || (parentLocked != nullptr && parentLocked->isChildOf(possibleParent));
    }

    std::shared_ptr<Node> Node::getRoot() {
        const auto parentSp = parent.lock();
        return !parentSp ? shared_from_this() : parentSp->getRoot();
    }

    Node::~Node() = default;

    bool MeshNode::isColorUserEditable() const {
        return true;
    }

    void LdrNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) {
        auto dummyColor = ldr::color_repo::getInstanceDummyColor();
        for (const auto& element: ldrFile->elements) {
            switch (element->getType()) {
                case 0: break;
                case 1: {
                    auto sfElement = std::dynamic_pointer_cast<ldr::SubfileReference>(element);
                    if (childrenWithOwnNode.find(sfElement) == childrenWithOwnNode.end()) {
                        mesh->addLdrSubfileReference(dummyColor, sfElement, glm::mat4(1.0f), windingInversed);
                    }
                } break;
                case 2:
                    mesh->addLdrLine(dummyColor, std::dynamic_pointer_cast<ldr::Line>(element), glm::mat4(1.0f));
                    break;
                case 3:
                    mesh->addLdrTriangle(dummyColor, std::dynamic_pointer_cast<ldr::Triangle>(element), glm::mat4(1.0f), windingInversed);
                    break;
                case 4:
                    mesh->addLdrQuadrilateral(dummyColor, std::dynamic_pointer_cast<ldr::Quadrilateral>(element), glm::mat4(1.0f), windingInversed);
                    break;
                case 5:
                    mesh->addLdrOptionalLine(dummyColor, std::dynamic_pointer_cast<ldr::OptionalLine>(element), glm::mat4(1.0f));
                    break;
            }
        }
    }

    std::shared_ptr<MpdSubfileNode> findMpdNodeAndAddSubfileNode(const std::shared_ptr<ldr::File>& ldrFile, ldr::ColorReference ldrColor, const std::shared_ptr<Node>& actualNode) {
        if (actualNode->getType() == TYPE_MULTI_PART_DOCUMENT) {
            //check if the subfileNode already exists
            for (const auto& child: actualNode->getChildren()) {
                if (child->getType() == TYPE_MPD_SUBFILE) {
                    auto mpdSubfileChild = std::dynamic_pointer_cast<MpdSubfileNode>(child);
                    if (ldrFile == mpdSubfileChild->ldrFile) {
                        return mpdSubfileChild;
                    }
                }
            }
            //the node doesn't exist, we have to create it
            auto addedNode = std::make_shared<MpdSubfileNode>(ldrFile, ldrColor, actualNode);
            addedNode->createChildNodes();
            actualNode->addChild(addedNode);
            return addedNode;
        } else if (actualNode->parent.expired()) {
            return nullptr;
        } else {
            return findMpdNodeAndAddSubfileNode(ldrFile, ldrColor, actualNode->parent.lock());
        }
    }

    LdrNode::LdrNode(NodeType nodeType, const std::shared_ptr<ldr::File>& ldrFile, const ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent) :
        MeshNode(ldrColor, parent), ldrFile(ldrFile) {
        type = nodeType;
        this->parent = parent;
        this->setColor(ldrColor);
        displayName = ldrFile->getDescription();
    }

    mesh_identifier_t LdrNode::getMeshIdentifier() const {
        return ldrFile->getHash();
    }

    std::string LdrNode::getDescription() {
        return ldrFile->getDescription();
    }

    bool LdrNode::isDisplayNameUserEditable() const {
        switch (ldrFile->metaInfo.type) {
            case ldr::MODEL:
            case ldr::MPD_SUBFILE: return true;
            default: return false;
        }
    }

    void LdrNode::addSubfileInstanceNode(const std::shared_ptr<ldr::File>& subFile, const ldr::ColorReference instanceColor) {
        auto subfileNode = findMpdNodeAndAddSubfileNode(subFile, {1}, shared_from_this());
        auto instanceNode = std::make_shared<MpdSubfileInstanceNode>(subfileNode, instanceColor, shared_from_this());
        this->children.push_back(instanceNode);
    }

    void LdrNode::createChildNodes() {
        plFunction();
        if (!childNodesCreated) {
            for (const auto& element: ldrFile->elements) {
                if (element->getType() == 1) {
                    auto sfElement = std::dynamic_pointer_cast<ldr::SubfileReference>(element);
                    auto subFile = sfElement->getFile();
                    std::shared_ptr<Node> newNode = nullptr;
                    if (subFile->metaInfo.type == ldr::MPD_SUBFILE) {
                        auto subFileNode = findMpdNodeAndAddSubfileNode(subFile, sfElement->color, shared_from_this());
                        newNode = std::make_shared<MpdSubfileInstanceNode>(subFileNode, sfElement->color, shared_from_this());
                        childrenWithOwnNode.insert(sfElement);
                    } else if (subFile->metaInfo.type == ldr::PART) {
                        childrenWithOwnNode.insert(sfElement);
                        newNode = std::make_shared<PartNode>(subFile, sfElement->color, shared_from_this());
                    }
                    if (nullptr != newNode) {
                        children.push_back(newNode);
                        newNode->setRelativeTransformation(sfElement->getTransformationMatrix());
                    }
                }
            }
            childNodesCreated = true;
        }
    }

    RootNode::RootNode() :
        Node(nullptr) {
        type = TYPE_ROOT;
        displayName = "Root";
        absoluteTransformation = relativeTransformation;
        absoluteTransformationValid = true;
    }

    bool RootNode::isDisplayNameUserEditable() const {
        return false;
    }

    mesh_identifier_t MpdSubfileInstanceNode::getMeshIdentifier() const {
        return mpdSubfileNode->getMeshIdentifier();
    }

    void MpdSubfileInstanceNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) {
        mpdSubfileNode->addToMesh(mesh, windingInversed);
    }

    std::string MpdSubfileInstanceNode::getDescription() {
        return mpdSubfileNode->getDescription();
    }

    bool MpdSubfileInstanceNode::isDisplayNameUserEditable() const {
        return false;
    }

    MpdSubfileInstanceNode::MpdSubfileInstanceNode(const std::shared_ptr<MpdSubfileNode>& mpdSubfileNode, ldr::ColorReference color, std::shared_ptr<Node> parent) :
        mpdSubfileNode(mpdSubfileNode), MeshNode(color, std::move(parent)) {
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

    MpdNode::MpdNode(const std::shared_ptr<ldr::File>& ldrFile, const ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent) :
        LdrNode(TYPE_MULTI_PART_DOCUMENT, ldrFile, ldrColor, parent) {
    }

    bool MpdSubfileNode::isDisplayNameUserEditable() const {
        return true;
    }

    MpdSubfileNode::MpdSubfileNode(const std::shared_ptr<ldr::File>& ldrFile, const ldr::ColorReference color, const std::shared_ptr<Node>& parent) :
        LdrNode(TYPE_MPD_SUBFILE, ldrFile, color, parent) {
        visible = false;
    }

    bool PartNode::isDisplayNameUserEditable() const {
        return false;
    }

    PartNode::PartNode(const std::shared_ptr<ldr::File>& ldrFile, const ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent) :
        LdrNode(TYPE_PART, ldrFile, ldrColor, parent) {
    }

    MeshNode::MeshNode(ldr::ColorReference color, std::shared_ptr<Node> parent) :
        Node(std::move(parent)), color(color) {
        type = TYPE_MESH;
    }

    ldr::ColorReference MeshNode::getDisplayColor() const {
        if (!parent.expired()) {
            const auto parentValue = parent.lock();
            if (color.get()->code == ldr::Color::MAIN_COLOR_CODE && (parentValue->getType() & TYPE_MESH) > 0) {
                return std::dynamic_pointer_cast<MeshNode>(parentValue)->getDisplayColor();
            }
        }
        return color;
    }

    void MeshNode::setColor(ldr::ColorReference newColor) {
        MeshNode::color = newColor;
    }

    ldr::ColorReference MeshNode::getElementColor() const {
        return color;
    }

    const char* getDisplayNameOfType(const NodeType& type) {
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

    color::RGB getColorOfType(const NodeType& type) {
        switch (type) {
            case TYPE_MPD_SUBFILE_INSTANCE: return config::get(config::COLOR_MPD_SUBFILE_INSTANCE);
            case TYPE_MPD_SUBFILE: return config::get(config::COLOR_MPD_SUBFILE);
            case TYPE_MULTI_PART_DOCUMENT: return config::get(config::COLOR_MULTI_PART_DOCUMENT);
            case TYPE_PART: return config::get(config::COLOR_OFFICAL_PART);//todo unoffical part
            case TYPE_OTHER:
            case TYPE_ROOT:
            case TYPE_MESH:
            case TYPE_LDRFILE:
            default: return color::RGB(255, 255, 255);
        }
    }

    std::shared_ptr<Node> getFirstSelectedNode(std::shared_ptr<Node> rootNode) {
        if (rootNode->selected) {
            return rootNode;
        }
        for (const auto& child: rootNode->getChildren()) {
            auto retVal = getFirstSelectedNode(child);
            if (retVal) {
                return retVal;
            }
        }
        return nullptr;
    }
}
#pragma clang diagnostic pop