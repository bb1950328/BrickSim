#include "editor.h"
#include <magic_enum.hpp>
#include <numeric>
#include <spdlog/spdlog.h>

#include "config.h"
#include "connection/engine.h"
#include "connection/visualization/connector_data_visualizer.h"
#include "controller.h"
#include "gui/graphical_transform/translation.h"
#include "ldr/file_repo.h"
#include "ldr/file_writer.h"
#include "palanteer.h"
#include "spdlog/fmt/bundled/format.h"

namespace bricksim {
    std::shared_ptr<Editor> Editor::createNew() {
        return std::make_shared<Editor>();
    }
    std::shared_ptr<Editor> Editor::openFile(const std::filesystem::path& path) {
        return std::make_shared<Editor>(path);
    }

    Editor::Editor() {
        const auto newFileLocation = util::extendHomeDirPath(config::get(config::NEW_FILE_LOCATION));
        const auto newName = getNameForNewLdrFile();
        filePath = newFileLocation / newName;
        fileNamespace = std::make_shared<ldr::FileNamespace>(newName, newFileLocation);
        init(ldr::file_repo::get().addLdrFileWithContent(fileNamespace, newName, *filePath, ldr::FileType::MODEL, ""));
    }

    Editor::Editor(const std::filesystem::path& path) :
        filePath(path),
        fileNamespace(std::make_shared<ldr::FileNamespace>(path.filename().string(), path.parent_path())) {
        init(ldr::file_repo::get().getFile(fileNamespace, path.filename().string()));
    }

    void Editor::init(const std::shared_ptr<ldr::File>& ldrFile) {
        rootNode = std::make_shared<etree::RootNode>();
        rootNode->displayName = ldrFile->metaInfo.name;
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
        return std::any_of(rootNode->getChildren().begin(), rootNode->getChildren().end(),
                           [this](auto item) {
                               return isModified(std::dynamic_pointer_cast<etree::ModelNode>(item));
                           });
    }
    bool Editor::isModified(const std::shared_ptr<etree::ModelNode>& model) const {
        const auto it = lastSavedVersions.find(model);
        return it == lastSavedVersions.end() || it->second < model->getVersion();
    }

    std::shared_ptr<graphics::Scene>& Editor::getScene() {
        return scene;
    }
    std::unique_ptr<graphical_transform::BaseAction>& Editor::getCurrentTransformAction() {
        return currentTransformAction;
    }

    void Editor::writeTo(const std::filesystem::path& mainFilePath) {
        plScope("Editor::writeTo");
        bool enableAutoReloadBackup = enableFileAutoReload;
        enableFileAutoReload = false;
        uomap_t<std::filesystem::path, std::vector<std::shared_ptr<ldr::File>>> ldrFilesByPath;
        for (const auto& item: rootNode->getChildren()) {
            const auto model = std::dynamic_pointer_cast<etree::ModelNode>(item);
            if (model != nullptr) {
                if (isModified(model)) {
                    model->writeChangesToLdrFile();
                    lastSavedVersions[model] = model->getVersion();
                }
                ldrFilesByPath[model->ldrFile->source.path].push_back(model->ldrFile);
            }
        }

        for (auto& [pathKey, filesValue]: ldrFilesByPath) {
            std::shared_ptr<ldr::File> mainFile = nullptr;
            for (auto it = filesValue.begin(); it != filesValue.end(); ++it) {
                if ((*it)->source.isMainFile) {
                    mainFile = *it;
                    filesValue.erase(it);
                    break;
                }
            }
            if (mainFile == nullptr) {
                spdlog::warn("attempting to write without mainFile to {}", pathKey.string());
                mainFile = std::make_shared<ldr::File>();
            }
            std::sort(filesValue.begin(),
                      filesValue.end(),
                      [](const auto& a, const auto& b) {
                          return a->metaInfo.name < b->metaInfo.name;
                      });
            const auto fileNamesList = std::accumulate(filesValue.begin(),
                                                       filesValue.end(),
                                                       std::string(),
                                                       [](std::string result, const auto& file) {
                                                           return file->source.isMainFile
                                                                          ? std::move(result)
                                                                          : (std::move(result) += file->metaInfo.name + ", ");
                                                       });

            std::filesystem::path physicalPath;
            if (!filePath->empty() && pathKey == *filePath) {
                //current file is the main file
                physicalPath = mainFilePath;
            } else {
                physicalPath = pathKey;
            }

            auto before = std::chrono::high_resolution_clock::now();
            ldr::writeFiles(mainFile, filesValue, physicalPath);
            auto after = std::chrono::high_resolution_clock::now();

            const auto timeUs = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count()) / 1000.f;
            spdlog::info("written {} files to {} in {}µs (mainFile={}, subfiles={})", 1 + filesValue.size(), pathKey.string(), timeUs, mainFile->metaInfo.name, fileNamesList);
        }
        enableFileAutoReload = enableAutoReloadBackup;
    }

    void Editor::save() {
        if (filePath->empty()) {
            throw std::invalid_argument("can't save when filePath is empty");
        }
        writeTo(*filePath);
    }

    void Editor::saveAs(const std::filesystem::path& newPath) {
        filePath = newPath;
        ldr::file_repo::get().changeFileName(fileNamespace, editingModel->ldrFile, newPath.filename().string());
        rootNode->displayName = newPath.filename().string();
        save();
    }

    void Editor::saveCopyAs(const std::filesystem::path& copyPath) {
        writeTo(copyPath);
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
        setAsActiveEditor();
    }

    void Editor::nodeSelectSet(const std::shared_ptr<etree::Node>& node) {
        for (const auto& selectedNode: selectedNodes) {
            selectedNode.first->selected = false;
        }
        selectedNodes.clear();
        node->selected = true;
        selectedNodes.emplace(node, node->getVersion());
        updateSelectionVisualization();
        setAsActiveEditor();
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
        setAsActiveEditor();
    }

    void Editor::nodeSelectAll() {
        nodeSelectNone();
        editingModel->selected = true;
        selectedNodes.emplace(editingModel, editingModel->getVersion());
        updateSelectionVisualization();
        setAsActiveEditor();
    }

    void Editor::nodeSelectNone() {
        for (const auto& item: selectedNodes) {
            item.first->selected = false;
        }
        selectedNodes.clear();
        updateSelectionVisualization();
        setAsActiveEditor();
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
        setAsActiveEditor();
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
    void Editor::centerElementIn3dView(const std::shared_ptr<etree::Node>& node) {
        const glm::mat4 transfT = glm::transpose(node->getAbsoluteTransformation());
        const glm::vec4 pos = transfT[3];
        spdlog::debug("targetPos=[{}, {}, {}]", pos.x, pos.y, pos.z);
        camera->setTargetPos(pos * constants::LDU_TO_OPENGL);
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
        if (nodeToDelete->getType() == etree::NodeType::TYPE_MODEL) {
            deleteModelInstances(std::dynamic_pointer_cast<etree::ModelNode>(nodeToDelete), rootNode);
        }
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
        if (ctrlPressed) {
            nodeSelectAddRemove(clickedNode);
        } else if (shiftPressed) {
            nodeSelectUntil(clickedNode);
        } else {
            nodeSelectSet(clickedNode);
        }
    }

    bool Editor::isNodeClickable(const std::shared_ptr<etree::Node>& node) {
        return true;
    }

    void Editor::endNodeTransformation() {
        if (currentTransformAction != nullptr) {
            currentTransformAction->end();
            currentTransformAction = nullptr;
        }
    }
    void Editor::startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType type) {
        std::optional<glm::svec2> realInitialCursorPos = std::nullopt;
        if (currentTransformAction != nullptr) {
            if (currentTransformAction->getType() == type) {
                return;
            } else {
                realInitialCursorPos = currentTransformAction->getInitialCursorPos();
                currentTransformAction->cancel();
                currentTransformAction = nullptr;
            }
        }
        std::vector<std::shared_ptr<etree::Node>> selectedNodesVec;
        selectedNodesVec.reserve(selectedNodes.size());
        std::transform(selectedNodes.cbegin(), selectedNodes.cend(),
                       std::back_inserter(selectedNodesVec),
                       [](const auto& item) { return item.first; });
        currentTransformAction = graphical_transform::createAction(type, *this, selectedNodesVec);
        if (realInitialCursorPos.has_value()) {
            currentTransformAction->start(*realInitialCursorPos);
        }
        updateCursorPos(cursorPos);
    }
    void Editor::updateCursorPos(const std::optional<glm::svec2>& value) {
        cursorPos = value;
        if (currentTransformAction != nullptr && cursorPos.has_value()) {
            if (currentTransformAction->getState() == graphical_transform::BaseAction::State::ACTIVE) {
                currentTransformAction->update(*cursorPos);
            } else {
                currentTransformAction->start(*cursorPos);
            }
        }
    }

    const std::shared_ptr<graphics::CadCamera>& Editor::getCamera() const {
        return camera;
    }
    const uomap_t<std::shared_ptr<etree::Node>, uint64_t>& Editor::getSelectedNodes() const {
        return selectedNodes;
    }

    std::string Editor::getFilename() const {
        return this->filePath->filename().string();
    }

    void Editor::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) {
        if (enableFileAutoReload) {
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
                if (meshNode != nullptr && meshNode->visible) {
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
            } else {
                aabb::AABB aabb;
                for (auto& node: selectedNodes) {
                    std::shared_ptr<etree::MeshNode> meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node.first);
                    if (meshNode != nullptr && meshNode->visible) {
                        auto nodeBBox = scene->getMeshCollection().getAbsoluteRotatedBBox(meshNode);
                        if (nodeBBox.has_value()) {
                            aabb.includeOBB(nodeBBox.value());
                        }
                    }
                }
                if (aabb.isDefined()) {
                    selectionVisualizationNode->visible = true;
                    glm::mat4 transf(1.f);
                    transf = glm::translate(transf, aabb.getCenter());
                    transf = glm::scale(transf, aabb.getSize() / 2.f);
                    selectionVisualizationNode->setRelativeTransformation(glm::transpose(transf));
                } else {
                }
            }
            selectionVisualizationNode->incrementVersion();
            for (auto& node: selectedNodes) {
                node.second = node.first->getVersion();
            }
        }
    }

    void Editor::update() {
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
    bool Editor::isActive() const {
        return controller::getActiveEditor().get() == this;
    }
    void Editor::setEditingModel(const std::shared_ptr<etree::ModelNode>& newEditingModel) {
        editingModel->visible = false;
        editingModel->incrementVersion();

        editingModel = newEditingModel;

        editingModel->visible = true;
        editingModel->incrementVersion();
    }
    void Editor::setAsActiveEditor() {
        controller::setActiveEditor(shared_from_this());
    }
    void Editor::deleteModelInstances(const std::shared_ptr<etree::ModelNode>& modelToDelete, const std::shared_ptr<etree::Node>& currentNode) {
        currentNode->removeChildIf([&modelToDelete](auto item) {
            return item->getType() == etree::NodeType::TYPE_MODEL_INSTANCE && std::dynamic_pointer_cast<etree::ModelInstanceNode>(item)->modelNode == modelToDelete;
        });
        for (const auto& child: currentNode->getChildren()) {
            deleteModelInstances(modelToDelete, child);
        }
    }
    void Editor::cancelNodeTransformation() {
        if (currentTransformAction != nullptr) {
            currentTransformAction->cancel();
            currentTransformAction = nullptr;
        }
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
