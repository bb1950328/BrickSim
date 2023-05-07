#include "editor.h"
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "config.h"
#include "connection/engine.h"
#include "connection/visualization/connector_data_visualizer.h"
#include "controller.h"
#include "ldr/file_repo.h"
#include "ldr/file_writer.h"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/fmt/ostr.h"

namespace bricksim {
    std::shared_ptr<Editor> Editor::createNew() {
        return std::make_shared<Editor>();
    }
    std::shared_ptr<Editor> Editor::openFile(const std::filesystem::path& path) {
        return std::make_shared<Editor>(path);
    }

    Editor::Editor() {
        const auto newName = getNameForNewLdrFile();
        fileNamespace = std::make_shared<ldr::FileNamespace>(newName, "/");
        init(ldr::file_repo::get().addLdrFileWithContent(fileNamespace, newName, ldr::FileType::MODEL, ""));
    }

    Editor::Editor(const std::filesystem::path& path) :
        filePath(path),
        fileNamespace(std::make_shared<ldr::FileNamespace>(path.filename().string(), path.parent_path())) {
        init(ldr::file_repo::get().getFile(fileNamespace, path.filename().string()));
    }

    void Editor::init(const std::shared_ptr<ldr::File>& ldrFile) {
        rootNode = std::make_shared<etree::RootNode>();
        editingModel = std::make_shared<etree::ModelNode>(ldrFile, 1, rootNode);
        rootNode->addChild(editingModel);
        editingModel->createChildNodes();
        editingModel->visible = true;
        editingModel->incrementVersion();

        const auto& allScenes = graphics::scenes::getAll();
        sceneId = graphics::scenes::FIRST_MAIN_SCENE_ID;
        while (allScenes.contains(sceneId)) {
            ++sceneId;
        }
        scene = graphics::scenes::create(sceneId);
        scene->setRootNode(rootNode);
        camera = std::make_shared<graphics::CadCamera>();
        scene->setCamera(camera);
        transformGizmo = std::make_unique<transform_gizmo::TransformGizmo>(*this);

        if (filePath.has_value()) {
            efsw::WatchID watchId = controller::getFileWatcher()->addWatch(filePath->parent_path().string(), this);
            if (magic_enum::enum_contains<efsw::Error>(static_cast<std::underlying_type_t<efsw::Errors::Error>>(watchId))) {
                spdlog::info("Cannot watch file \"{}\": {}", filePath->string(), magic_enum::enum_name(static_cast<efsw::Error>(watchId)));
            } else {
                fileWatchId = watchId;
            }
        }

        if (config::get(config::DISPLAY_CONNECTOR_DATA_IN_3D_VIEW)) {
            addConnectorDataVisualization(rootNode);
        }
    }
    void Editor::addConnectorDataVisualization(const std::shared_ptr<etree::Node>& node) const {
        for (const auto& child: node->getChildren()) {
            if (child->type == etree::NodeType::TYPE_PART) {
                const auto& fileName = std::dynamic_pointer_cast<etree::PartNode>(child)->ldrFile->metaInfo.name;
                connection::visualization::addVisualization(fileName, child);
            }
            addConnectorDataVisualization(child);
        }
    }

    std::string Editor::getNameForNewLdrFile() {
        static uint32_t nameCounter = 0;
        std::string name;
        auto& fileRepo = ldr::file_repo::get();
        do {
            nameCounter++;
            name = fmt::format("Untitled{:d}.mpd", nameCounter);
        } while (fileRepo.getNamespace(name) != nullptr);
        return name;
    }

    Editor::~Editor() {
        graphics::scenes::remove(sceneId);
        if (fileWatchId.has_value()) {
            controller::getFileWatcher()->removeWatch(fileWatchId.value());
        }
    }

    const std::optional<std::filesystem::path>& Editor::getFilePath() {
        return filePath;
    }

    std::shared_ptr<etree::RootNode>& Editor::getRootNode() {
        return rootNode;
    }

    std::shared_ptr<etree::ModelNode>& Editor::getEditingModel() {
        return editingModel;
    }

    bool Editor::isModified() const {
        return lastSavedVersion != editingModel->getVersion();
    }

    std::shared_ptr<graphics::Scene>& Editor::getScene() {
        return scene;
    }
    std::unique_ptr<transform_gizmo::TransformGizmo>& Editor::getTransformGizmo() {
        return transformGizmo;
    }
    void Editor::save() {
        if (filePath->empty()) {
            throw std::invalid_argument("can't save when filePath is empty");
        }
        if (lastSavedVersion != editingModel->getVersion()) {
            editingModel->writeChangesToLdrFile();
            ldr::writeFile(editingModel->ldrFile, filePath.value());
            lastSavedVersion = editingModel->getVersion();
        }
    }

    void Editor::saveAs(const std::filesystem::path& newPath) {
        filePath = newPath;
        ldr::file_repo::get().changeFileName(fileNamespace, editingModel->ldrFile, newPath.filename().string());
        save();
    }

    void Editor::saveCopyAs(const std::filesystem::path& copyPath) {
        editingModel->writeChangesToLdrFile();
        ldr::writeFile(editingModel->ldrFile, copyPath);
    }

    void Editor::nodeSelectAddRemove(const std::shared_ptr<etree::Node>& node) {
        auto iterator = selectedNodes.find(node);
        node->selected = iterator == selectedNodes.end();
        if (node->selected) {
            selectedNodes.emplace(node, node->getVersion());
        } else {
            selectedNodes.erase(iterator);
        }
        updateSelectionVisualization();
    }

    void Editor::nodeSelectSet(const std::shared_ptr<etree::Node>& node) {
        for (const auto& selectedNode: selectedNodes) {
            selectedNode.first->selected = false;
        }
        selectedNodes.clear();
        node->selected = true;
        selectedNodes.emplace(node, node->getVersion());
        updateSelectionVisualization();
    }

    void Editor::nodeSelectUntil(const std::shared_ptr<etree::Node>& node) {
        auto rangeActive = false;
        auto keepGoing = true;
        const auto& parentChildren = node->parent.lock()->getChildren();
        for (auto iterator = parentChildren.rbegin();
             iterator != parentChildren.rend() && keepGoing;
             iterator++) {
            auto itNode = *iterator;
            if (itNode == node || itNode->selected) {
                if (rangeActive) {
                    keepGoing = false;
                } else {
                    rangeActive = true;
                }
            }
            if (rangeActive) {
                itNode->selected = true;
                selectedNodes.emplace(itNode, itNode->getVersion());
            }
        }
        updateSelectionVisualization();
    }

    void Editor::nodeSelectAll() {
        nodeSelectNone();
        editingModel->selected = true;
        selectedNodes.emplace(editingModel, editingModel->getVersion());
        updateSelectionVisualization();
    }

    void Editor::nodeSelectNone() {
        for (const auto& item: selectedNodes) {
            item.first->selected = false;
        }
        selectedNodes.clear();
        updateSelectionVisualization();
    }

    void Editor::nodeSelectConnected() {
        connection::ConnectionGraph graph;
        for (const auto& item: selectedNodes) {
            const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(item.first);
            if (ldrNode != nullptr) {
                connection::engine::findConnections(ldrNode, editingModel, graphics::scenes::get(sceneId)->getMeshCollection(), graph);
            }
        }
        for (const auto& item: selectedNodes) {
            const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(item.first);
            if (ldrNode != nullptr) {
                for (const auto& edge: graph.getConnections(ldrNode)) {
                    if (!edge.second.empty()) {
                        selectedNodes.emplace(edge.first, edge.first->getVersion());
                    }
                }
            }
        }
        updateSelectionVisualization();
    }

    void Editor::setStandard3dView(int i) {
        camera->setStandardView(i);
    }

    void Editor::rotateViewUp() {
        camera->mouseRotate(0, -1);
    }
    void Editor::rotateViewDown() {
        camera->mouseRotate(0, +1);
    }
    void Editor::rotateViewLeft() {
        camera->mouseRotate(-1, 0);
    }
    void Editor::rotateViewRight() {
        camera->mouseRotate(+1, 0);
    }
    void Editor::panViewUp() {
        camera->mousePan(0, -1);
    }
    void Editor::panViewDown() {
        camera->mousePan(0, +1);
    }
    void Editor::panViewLeft() {
        camera->mousePan(-1, 0);
    }
    void Editor::panViewRight() {
        camera->mousePan(+1, 0);
    }

    void Editor::insertLdrElement(const std::shared_ptr<ldr::File>& ldrFile) {
        switch (ldrFile->metaInfo.type) {
            case ldr::FileType::MPD_SUBFILE:
                editingModel->addModelInstanceNode(ldrFile, {1});
                editingModel->incrementVersion();
                break;
            case ldr::FileType::PART:
                editingModel->addChild(std::make_shared<etree::PartNode>(ldrFile, ldr::ColorReference{1}, editingModel, nullptr));
                editingModel->incrementVersion();
                break;
            case ldr::FileType::SUBPART:
            case ldr::FileType::PRIMITIVE:
            default: break;
        }
    }

    void Editor::deleteElement(const std::shared_ptr<etree::Node>& nodeToDelete) {
        auto parent = nodeToDelete->parent.lock();
        parent->removeChild(nodeToDelete);
        parent->incrementVersion();
        selectedNodes.erase(nodeToDelete);
        updateSelectionVisualization();
    }

    void Editor::deleteSelectedElements() {
        for (const auto& item: selectedNodes) {
            deleteElement(item.first);
        }
        selectedNodes.clear();
        updateSelectionVisualization();
    }

    void Editor::hideSelectedElements() {
        for (const auto& item: selectedNodes) {
            item.first->visible = false;
        }
    }

    void unhideElementRecursively(const std::shared_ptr<etree::Node>& node) {
        node->visible = false;
        for (const auto& child: node->getChildren()) {
            unhideElementRecursively(child);
        }
    }

    void Editor::unhideAllElements() {
        unhideElementRecursively(rootNode);
    }

    void Editor::nodeClicked(const std::shared_ptr<etree::Node>& clickedNode, bool ctrlPressed, bool shiftPressed) {
        if (transformGizmo->ownsNode(clickedNode)) {
            //todo transformGizmo->nodeClicked
        } else {
            if (ctrlPressed) {
                nodeSelectAddRemove(clickedNode);
            } else if (shiftPressed) {
                nodeSelectUntil(clickedNode);
            } else {
                nodeSelectSet(clickedNode);
            }
        }
    }

    bool Editor::isNodeClickable(const std::shared_ptr<etree::Node>& node) {
        return !transformGizmo->ownsNode(node);
    }

    bool Editor::isNodeDraggable(const std::shared_ptr<etree::Node>& node) {
        return transformGizmo->ownsNode(node);
    }

    void Editor::startNodeDrag(std::shared_ptr<etree::Node>& draggedNode, const glm::svec2& initialCursorPos) {
        if (transformGizmo->ownsNode(draggedNode)) {
            transformGizmo->startDrag(draggedNode, initialCursorPos);
            currentlyDraggingNodeType = DraggingNodeType::TRANSFORM_GIZMO;
        }
    }

    void Editor::updateNodeDragDelta(glm::usvec2 delta) {
        switch (currentlyDraggingNodeType) {
            case DraggingNodeType::TRANSFORM_GIZMO:
                transformGizmo->updateCurrentDragDelta(delta);
                break;
            case DraggingNodeType::NONE:
            default:
                break;
        }
    }

    void Editor::endNodeDrag() {
        switch (currentlyDraggingNodeType) {
            case DraggingNodeType::TRANSFORM_GIZMO:
                transformGizmo->endDrag();
                break;
            case DraggingNodeType::NONE:
            default:
                break;
        }
    }
    const std::shared_ptr<graphics::CadCamera>& Editor::getCamera() const {
        return camera;
    }
    const uomap_t<std::shared_ptr<etree::Node>, uint64_t>& Editor::getSelectedNodes() const {
        return selectedNodes;
    }

    const std::string& Editor::getFilename() const {
        return editingModel->ldrFile->metaInfo.name;
    }

    void Editor::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) {
        auto fullPath = std::filesystem::path(dir) / filename;
        if (filePath.has_value() && fullPath == filePath.value()) {
            spdlog::info(R"(editor file change detected: {} fullPath="{}" oldFilename="{}")",
                         magic_enum::enum_name(action), fullPath.string(), oldFilename);
            rootNode->removeChild(editingModel);
            editingModel = std::make_shared<etree::ModelNode>(ldr::file_repo::get().reloadFile(fileNamespace, filePath->string()), 1, rootNode);
            editingModel->createChildNodes();
            rootNode->addChild(editingModel);
            editingModel->visible = true;
            editingModel->incrementVersion();
        }
    }

    void Editor::updateSelectionVisualization() {
        if (selectedNodes.empty()) {
            if (selectionVisualizationNode != nullptr && selectionVisualizationNode->visible) {
                selectionVisualizationNode->visible = false;
                selectionVisualizationNode->incrementVersion();
            }
        } else {
            if (selectionVisualizationNode == nullptr) {
                selectionVisualizationNode = std::make_shared<SelectionVisualizationNode>(rootNode);
                rootNode->addChild(selectionVisualizationNode);
            }
            selectionVisualizationNode->visible = false;

            if (selectedNodes.size() == 1) {
                const auto meshNode = std::dynamic_pointer_cast<etree::MeshNode>(selectedNodes.begin()->first);
                if (meshNode != nullptr) {
                    const auto relativeAABB = scene->getMeshCollection().getRelativeAABB(meshNode);
                    if (relativeAABB.isDefined()) {
                        const auto rotatedBBox = aabb::OBB(relativeAABB, glm::vec3(0.f, 0.f, 0.f),
                                                           glm::quat(1.f, 0.f, 0.f, 0.f))
                                                         .transform(
                                                                 glm::transpose(meshNode->getAbsoluteTransformation()));
                        selectionVisualizationNode->visible = true;
                        selectionVisualizationNode->setRelativeTransformation(
                                glm::transpose(rotatedBBox.getUnitBoxTransformation()));
                    }
                }
                selectedNodes.begin()->second = selectedNodes.begin()->first->getVersion();
            } else {
                aabb::AABB aabb;
                for (auto& node: selectedNodes) {
                    std::shared_ptr<etree::MeshNode> meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node.first);
                    if (meshNode != nullptr) {
                        auto nodeBBox = scene->getMeshCollection().getAbsoluteRotatedBBox(meshNode);
                        if (nodeBBox.has_value()) {
                            aabb.includeOBB(nodeBBox.value());
                        }
                    }
                    node.second = node.first->getVersion();
                }
                if (aabb.isDefined()) {
                    selectionVisualizationNode->visible = true;
                    glm::mat4 transf(1.f);
                    transf = glm::translate(transf, aabb.getCenter());
                    transf = glm::scale(transf, aabb.getSize() / 2.f);
                    spdlog::debug("center={}, size={}", aabb.getCenter(), aabb.getSize());
                    selectionVisualizationNode->setRelativeTransformation(glm::transpose(transf));
                } else {
                }
            }
            selectionVisualizationNode->incrementVersion();
        }
    }

    void Editor::update() {
        transformGizmo->update();
        for (const auto& item: selectedNodes) {
            if (item.first->getVersion() != item.second) {
                updateSelectionVisualization();
                break;
            }
        }
    }
    void Editor::inlineElement(const std::shared_ptr<etree::Node>& nodeToInline) {
        inlineElement(nodeToInline, true);
    }
    void Editor::inlineElement(const std::shared_ptr<etree::Node>& nodeToInline, bool updateSelectionVisualization) {
        auto parent = nodeToInline->parent.lock();
        const auto& siblings = parent->getChildren();
        std::size_t indexInSiblings = 0;
        while (siblings[indexInSiblings] != nodeToInline && indexInSiblings < siblings.size()) {
            ++indexInSiblings;
        }

        const bool nodeWasSelected = selectedNodes.erase(nodeToInline) > 0;

        const auto modelInstNodeToInline = std::dynamic_pointer_cast<etree::ModelInstanceNode>(nodeToInline);
        if (modelInstNodeToInline != nullptr) {
            for (const auto& item: modelInstNodeToInline->modelNode->getChildren()) {
                const auto meshItem = std::dynamic_pointer_cast<etree::MeshNode>(item);
                const auto partItem = std::dynamic_pointer_cast<etree::PartNode>(item);
                std::shared_ptr<etree::Node> newNode = nullptr;
                const auto newColor = meshItem->getElementColor() == ldr::Color::MAIN_COLOR_CODE
                                              ? modelInstNodeToInline->getElementColor()
                                              : meshItem->getElementColor();
                if (partItem != nullptr) {
                    newNode = std::make_shared<etree::PartNode>(partItem->ldrFile,
                                                                newColor,
                                                                parent,
                                                                modelInstNodeToInline->getDirectTexmap());
                } else {
                    const auto subfileInstItem = std::dynamic_pointer_cast<etree::ModelInstanceNode>(item);
                    if (subfileInstItem != nullptr) {
                        newNode = std::make_shared<etree::ModelInstanceNode>(subfileInstItem->modelNode,
                                                                             newColor,
                                                                             parent,
                                                                             subfileInstItem->getDirectTexmap());
                    }
                }
                if (newNode != nullptr) {
                    const auto meshT = glm::transpose(meshItem->getRelativeTransformation());
                    const auto nodeToInlineT = glm::transpose(nodeToInline->getRelativeTransformation());
                    newNode->setRelativeTransformation(glm::transpose(nodeToInlineT * meshT));
                    parent->addChild(indexInSiblings++, newNode);
                    if (nodeWasSelected) {
                        selectedNodes.emplace(newNode, newNode->getVersion());
                    }
                }
            }
        } else {
            spdlog::warn("nothing to inline on node {}", nodeToInline->getDescription());
            return;
        }
        parent->removeChild(nodeToInline);
        parent->incrementVersion();
        if (nodeWasSelected && updateSelectionVisualization) {
            this->updateSelectionVisualization();
        }
    }

    void Editor::inlineSelectedElements() {
        std::vector<std::shared_ptr<etree::Node>> nodesToInline;
        for (const auto& [node, version]: selectedNodes) {
            nodesToInline.push_back(node);
        }
        for (const auto& node: nodesToInline) {
            inlineElement(node, false);
        }
        updateSelectionVisualization();
    }
    std::shared_ptr<ldr::FileNamespace>& Editor::getFileNamespace() {
        return fileNamespace;
    }

    SelectionVisualizationNode::SelectionVisualizationNode(const std::shared_ptr<Node>& parent) :
        MeshNode(1, parent, nullptr) {
        visibleInElementTree = false;
    }

    mesh_identifier_t SelectionVisualizationNode::getMeshIdentifier() const {
        return constants::MESH_ID_SELECTION_VISUALIZATION;
    }

    void SelectionVisualizationNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed,
                                               const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        auto& lineData = mesh->getLineData();
        glm::vec3 color(0, 0, 1);
        //square z=-1
        lineData.addVertex({glm::vec3(-1, -1, -1), color});
        lineData.addVertex({glm::vec3(+1, -1, -1), color});

        lineData.addVertex({glm::vec3(-1, -1, -1), color});
        lineData.addVertex({glm::vec3(-1, +1, -1), color});

        lineData.addVertex({glm::vec3(+1, -1, -1), color});
        lineData.addVertex({glm::vec3(+1, +1, -1), color});

        lineData.addVertex({glm::vec3(-1, +1, -1), color});
        lineData.addVertex({glm::vec3(+1, +1, -1), color});

        //square z=1
        lineData.addVertex({glm::vec3(-1, -1, +1), color});
        lineData.addVertex({glm::vec3(+1, -1, +1), color});

        lineData.addVertex({glm::vec3(-1, -1, +1), color});
        lineData.addVertex({glm::vec3(-1, +1, +1), color});

        lineData.addVertex({glm::vec3(+1, -1, +1), color});
        lineData.addVertex({glm::vec3(+1, +1, +1), color});

        lineData.addVertex({glm::vec3(-1, +1, +1), color});
        lineData.addVertex({glm::vec3(+1, +1, +1), color});

        //vertical
        lineData.addVertex({glm::vec3(-1, -1, -1), color});
        lineData.addVertex({glm::vec3(-1, -1, +1), color});

        lineData.addVertex({glm::vec3(+1, -1, -1), color});
        lineData.addVertex({glm::vec3(+1, -1, +1), color});

        lineData.addVertex({glm::vec3(-1, +1, -1), color});
        lineData.addVertex({glm::vec3(-1, +1, +1), color});

        lineData.addVertex({glm::vec3(+1, +1, -1), color});
        lineData.addVertex({glm::vec3(+1, +1, +1), color});
    }

    bool SelectionVisualizationNode::isDisplayNameUserEditable() const {
        return false;
    }
}
