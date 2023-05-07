#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
#include "element_tree.h"
#include "config.h"
#include "ldr/file_repo.h"
#include <glm/gtx/normal.hpp>
#include <magic_enum.hpp>
#include <palanteer.h>
#include <spdlog/spdlog.h>
#include <utility>

#ifdef RGB
    #undef RGB
#endif

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
        type = NodeType::TYPE_OTHER;
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

    void Node::addChild(std::size_t position, const std::shared_ptr<Node>& newChild) {
        children.insert(children.cbegin() + position, newChild);
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

    std::shared_ptr<RootNode> Node::getRoot() {
        const auto parentSp = parent.lock();
        return parentSp == nullptr
                       ? std::dynamic_pointer_cast<RootNode>(shared_from_this())
                       : parentSp->getRoot();
    }

    uint64_t Node::getVersion() const {
        return version;
    }

    void Node::incrementVersion() {
        Node* node = this;
        do {
            ++node->version;
            node = node->parent.lock().get();
        } while (node != nullptr);
    }
    bool Node::isDirectChildOfTypeAllowed(NodeType type) const {
        return true;
    }

    Node::~Node() = default;

    bool MeshNode::isColorUserEditable() const {
        return true;
    }

    void LdrNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        auto dummyColor = ldr::color_repo::getInstanceDummyColor();
        for (const auto& element: ldrFile->elements) {
            if (element->hidden) {
                continue;
            }
            switch (element->getType()) {
                case 0: break;
                case 1: {
                    auto sfElement = std::dynamic_pointer_cast<ldr::SubfileReference>(element);
                    if (childrenWithOwnNode.find(sfElement) == childrenWithOwnNode.end()) {
                        mesh->addLdrSubfileReference(ldrFile->nameSpace, dummyColor, sfElement, glm::mat4(1.0f), windingInversed, texmap);
                    }
                } break;
                case 2:
                    mesh->addLdrLine(dummyColor, std::dynamic_pointer_cast<ldr::Line>(element), glm::mat4(1.0f));
                    break;
                case 3:
                    mesh->addLdrTriangle(dummyColor, std::dynamic_pointer_cast<ldr::Triangle>(element), glm::mat4(1.0f), windingInversed, texmap);
                    break;
                case 4:
                    mesh->addLdrQuadrilateral(dummyColor, std::dynamic_pointer_cast<ldr::Quadrilateral>(element), glm::mat4(1.0f), windingInversed, texmap);
                    break;
                case 5:
                    mesh->addLdrOptionalLine(dummyColor, std::dynamic_pointer_cast<ldr::OptionalLine>(element), glm::mat4(1.0f));
                    break;
            }
        }
    }

    LdrNode::LdrNode(NodeType nodeType, const std::shared_ptr<ldr::File>& ldrFile, const ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent, const std::shared_ptr<ldr::TexmapStartCommand>& directTexmap) :
        MeshNode(ldrColor, parent, directTexmap), ldrFile(ldrFile) {
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
            case ldr::FileType::MODEL:
            case ldr::FileType::MPD_SUBFILE: return true;
            default: return false;
        }
    }

    void LdrNode::createChildNodes() {
        plFunction();
        if (!childNodesCreated) {
            for (const auto& element: ldrFile->elements) {
                if (element->hidden) {
                    continue;
                }
                if (element->getType() == 0) {
                    const auto texmapStartCommand = dynamic_pointer_cast<ldr::TexmapStartCommand>(element);
                    if (texmapStartCommand != nullptr) {
                        children.push_back(std::make_shared<TexmapNode>(ldrFile->nameSpace, texmapStartCommand, shared_from_this()));
                    }
                } else if (element->getType() == 1) {
                    auto sfElement = std::dynamic_pointer_cast<ldr::SubfileReference>(element);
                    auto subFile = sfElement->getFile(ldrFile->nameSpace);

                    if (subFile->metaInfo.type == ldr::FileType::MPD_SUBFILE || subFile->metaInfo.type == ldr::FileType::MODEL || subFile->metaInfo.type == ldr::FileType::PART) {
                        const auto newNode = addModelInstanceNode(subFile, sfElement->color);
                        newNode->setRelativeTransformation(sfElement->getTransformationMatrix());
                        subfileRefChildNodeSaveInfos.emplace(newNode, ChildNodeSaveInfo{newNode->getVersion(), element});
                    }
                }
            }
            childNodesCreated = true;
        }
    }

    std::shared_ptr<MeshNode> LdrNode::addModelInstanceNode(const std::shared_ptr<ldr::File>& subFile, ldr::ColorReference instanceColor) {
        std::shared_ptr<MeshNode> newNode;
        if (subFile->metaInfo.type == ldr::FileType::PART) {
            newNode = std::make_shared<PartNode>(subFile, instanceColor, shared_from_this(), nullptr);
        } else {
            const auto subModelNode = getRoot()->getModelNode(subFile);
            newNode = std::make_shared<ModelInstanceNode>(subModelNode, instanceColor, shared_from_this(), nullptr);
        }
        addChild(newNode);
        return newNode;
    }
    void LdrNode::writeChangesToLdrFile() {
        if (version != lastSaveToLdrFileVersion) {
            for (const auto& item: children) {
                if (item->getType() == NodeType::TYPE_PART || item->getType() == NodeType::TYPE_MODEL_INSTANCE) {
                    auto saveInfos = subfileRefChildNodeSaveInfos.find(item);
                    if (saveInfos != subfileRefChildNodeSaveInfos.end()) {
                        if (item->getVersion() != saveInfos->second.lastSaveToLdrFileVersion) {
                            const auto& ldrElement = saveInfos->second.ldrElement;
                            auto subfileRefElement = std::dynamic_pointer_cast<ldr::SubfileReference>(ldrElement);
                            subfileRefElement->setTransformationMatrix(item->getRelativeTransformation());
                        }
                    } else {
                        auto meshItem = std::dynamic_pointer_cast<MeshNode>(item);
                        auto subfileRefElement = std::make_shared<ldr::SubfileReference>(meshItem->getElementColor(), item->getRelativeTransformation(), false);
                        subfileRefElement->setTransformationMatrix(item->getRelativeTransformation());
                        subfileRefElement->step = ldrFile->elements.back()->step;
                        ldrFile->elements.push_back(subfileRefElement);
                        subfileRefChildNodeSaveInfos.emplace(item, ChildNodeSaveInfo{item->getVersion(), subfileRefElement});
                    }
                }
            }
            lastSaveToLdrFileVersion = version;
        }
    }

    RootNode::RootNode() :
        Node(nullptr) {
        type = NodeType::TYPE_ROOT;
        displayName = "Root";
        absoluteTransformation = relativeTransformation;
        absoluteTransformationValid = true;
    }

    bool RootNode::isDisplayNameUserEditable() const {
        return false;
    }
    bool RootNode::isDirectChildOfTypeAllowed(NodeType type) const {
        return type == NodeType::TYPE_MESH || type == NodeType::TYPE_MODEL;
    }
    std::shared_ptr<ModelNode> RootNode::getModelNode(const std::shared_ptr<ldr::File>& ldrFile) {
        for (const auto& item: children) {
            auto modelItem = dynamic_pointer_cast<ModelNode>(item);
            if (modelItem != nullptr && modelItem->ldrFile == ldrFile) {
                return modelItem;
            }
        }
        auto newNode = std::make_shared<ModelNode>(ldrFile, ldr::color_repo::INSTANCE_DUMMY_COLOR_CODE, shared_from_this());
        newNode->createChildNodes();
        addChild(newNode);
        return newNode;
    }

    mesh_identifier_t ModelInstanceNode::getMeshIdentifier() const {
        return modelNode->getMeshIdentifier();
    }

    void ModelInstanceNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        modelNode->addToMesh(mesh, windingInversed, texmap);
    }

    std::string ModelInstanceNode::getDescription() {
        return modelNode->getDescription();
    }

    bool ModelInstanceNode::isDisplayNameUserEditable() const {
        return false;
    }

    ModelInstanceNode::ModelInstanceNode(const std::shared_ptr<ModelNode>& modelNode, ldr::ColorReference color, const std::shared_ptr<Node>& parent, const std::shared_ptr<ldr::TexmapStartCommand>& directTexmap) :
        MeshNode(color, parent, directTexmap), modelNode(modelNode) {
        type = NodeType::TYPE_MODEL_INSTANCE;
        this->displayName = modelNode->displayName;
    }

    bool ModelNode::isDisplayNameUserEditable() const {
        return true;
    }

    ModelNode::ModelNode(const std::shared_ptr<ldr::File>& ldrFile, const ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent) :
        LdrNode(NodeType::TYPE_MODEL, ldrFile, ldrColor, parent, nullptr) {
        visible = false;
    }

    bool PartNode::isDisplayNameUserEditable() const {
        return false;
    }

    PartNode::PartNode(const std::shared_ptr<ldr::File>& ldrFile, const ldr::ColorReference ldrColor, const std::shared_ptr<Node>& parent, const std::shared_ptr<ldr::TexmapStartCommand>& directTexmap) :
        LdrNode(NodeType::TYPE_PART, ldrFile, ldrColor, parent, directTexmap) {
    }

    MeshNode::MeshNode(ldr::ColorReference color, const std::shared_ptr<Node>& parent, std::shared_ptr<ldr::TexmapStartCommand> directTexmap) :
        Node(parent), color(color), directTexmap(std::move(directTexmap)) {
        type = NodeType::TYPE_MESH;
    }

    ldr::ColorReference MeshNode::getDisplayColor() const {
        if (!parent.expired()) {
            const auto parentValue = parent.lock();
            if (color.get()->code == ldr::Color::MAIN_COLOR_CODE && (static_cast<uint32_t>(parentValue->getType()) & static_cast<uint32_t>(NodeType::TYPE_MESH)) > 0) {
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

    const std::shared_ptr<ldr::TexmapStartCommand>& MeshNode::getAppliedTexmap() const {
        if (directTexmap != nullptr) {
            return directTexmap;
        }
        auto parentAsMeshNode = std::dynamic_pointer_cast<MeshNode>(parent.lock());
        if (parentAsMeshNode != nullptr) {
            return parentAsMeshNode->getAppliedTexmap();
        }

        const static std::shared_ptr<ldr::TexmapStartCommand> null;
        return null;
    }
    const std::shared_ptr<ldr::TexmapStartCommand>& MeshNode::getDirectTexmap() const {
        return directTexmap;
    }

    const char* getDisplayNameOfType(const NodeType& type) {
        switch (type) {
            case NodeType::TYPE_ROOT: return "Root";
            case NodeType::TYPE_MESH: return "Mesh";
            case NodeType::TYPE_MODEL_INSTANCE: return "Model Instance";
            case NodeType::TYPE_LDRFILE: return "LDraw file";
            case NodeType::TYPE_MODEL: return "Model";
            case NodeType::TYPE_PART: return "Part";
            case NodeType::TYPE_OTHER:
            default: return "Other";
        }
    }

    color::RGB getColorOfType(const NodeType& type) {
        switch (type) {
            case NodeType::TYPE_MODEL_INSTANCE: return config::get(config::COLOR_MPD_SUBFILE_INSTANCE);
            case NodeType::TYPE_MODEL: return config::get(config::COLOR_MULTI_PART_DOCUMENT);
            case NodeType::TYPE_PART: return config::get(config::COLOR_OFFICAL_PART);//todo unoffical part
            case NodeType::TYPE_OTHER:
            case NodeType::TYPE_ROOT:
            case NodeType::TYPE_MESH:
            case NodeType::TYPE_LDRFILE:
            default: return {255, 255, 255};
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

    TexmapNode::TexmapNode(const std::shared_ptr<ldr::FileNamespace>& fileNamespace, const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, const std::shared_ptr<Node>& parent) :
        MeshNode(ldr::color_repo::INSTANCE_DUMMY_COLOR_CODE, parent, nullptr),
        projectionMethod(startCommand->projectionMethod),
        p1(startCommand->x1(), startCommand->y1(), startCommand->z1()),
        p2(startCommand->x2(), startCommand->y2(), startCommand->z2()),
        p3(startCommand->x3(), startCommand->y3(), startCommand->z3()),
        textureFilename(startCommand->textureFilename),
        a(startCommand->a()),
        b(startCommand->b()) {
        auto flipVerticallyBackup = util::isStbiFlipVertically();
        util::setStbiFlipVertically(false);//todo I'm not 100% sure if this is right
        const auto& textureFile = ldr::file_repo::get().getBinaryFile(fileNamespace, textureFilename, ldr::file_repo::BinaryFileSearchPath::TEXMAP);
        texture = graphics::Texture::getFromBinaryFileCached(textureFile);
        util::setStbiFlipVertically(flipVerticallyBackup);
        updateCalculatedValues();
    }

    void TexmapNode::updateCalculatedValues() {
        displayName = fmt::format("Texmap {} {}", magic_enum::enum_name(projectionMethod), textureFilename);
        //todo modify directTexmap and increment version
        if (projectionMethod == ldr::TexmapStartCommand::ProjectionMethod::PLANAR) {
            /*const glm::vec3 scale{glm::length(p2 - p1), glm::length(p3 - p1), 1.f};
            glm::mat4 transf = glm::scale(glm::mat4(1.f), scale);
            //now the plane has the correct size

            const glm::vec3 untransformedPlaneNormal{0.f, 0.f, 1.f};
            const auto transformedPlaneNormal = glm::triangleNormal(p1, p2, p3);//todo find out which one is the correct winding order
            const auto rotation = util::quaternionRotationFromOneVectorToAnother(untransformedPlaneNormal, transformedPlaneNormal);
            transf = glm::toMat4(rotation) * transf;
            //now the plane normal points to the right direction


            //const auto p4 = p2 + p3 - p1;
            const auto center = .5f * (p2 + p3);
            transf = glm::translate(glm::mat4(1.f), center) * transf;
            //now the center of the plane is also at the right place


            const glm::vec3 intermediateP1 = glm::vec4(-.5f, -.5f, 0.f, 1.f) * transf;
            const auto rotation2 = util::quaternionRotationFromOneVectorToAnother(intermediateP1-center, p1-center);
            transf = glm::toMat4(rotation2) * transf;
            //now the plane is also rotated around its normal correctly


            setRelativeTransformation(glm::transpose(transf));*/
        }
        //setRelativeTransformation(glm::translate(glm::mat4(1.0f), p1));
    }

    mesh_identifier_t TexmapNode::getMeshIdentifier() const {
        if (projectionMethod == ldr::TexmapStartCommand::ProjectionMethod::PLANAR) {
            return util::combinedHash(projectionMethod, textureFilename);
        } else if (projectionMethod == ldr::TexmapStartCommand::ProjectionMethod::SPHERICAL) {
            return util::combinedHash(projectionMethod, textureFilename, a, glm::length2(p3 - p1));
        } else {
            return util::combinedHash(projectionMethod, textureFilename, a, b, glm::length2(p2 - p1));
        }
    }

    void TexmapNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
    }

    bool TexmapNode::isDisplayNameUserEditable() const {
        return false;
    }
}
#pragma clang diagnostic pop
