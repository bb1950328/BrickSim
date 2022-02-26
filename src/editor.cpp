#include "editor.h"
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

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

    Editor::Editor() :
        filePath() {
        init(ldr::file_repo::get().addLdrFileWithContent(getNameForNewLdrFile(), ldr::MODEL, ""));
    }

    Editor::Editor(const std::filesystem::path& path) :
        filePath(path) {
        init(ldr::file_repo::get().getFile(path.string()));
    }

    void Editor::init(const std::shared_ptr<ldr::File>& ldrFile) {
        rootNode = std::make_shared<etree::RootNode>();
        documentNode = std::make_shared<etree::MpdNode>(ldrFile, 1, rootNode);
        documentNode->createChildNodes();
        rootNode->addChild(documentNode);
        documentNode->incrementVersion();

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
            if (magic_enum::enum_contains<efsw::Error>(watchId)) {
                spdlog::info("Cannot watch file \"{}\": {}", filePath->string(), magic_enum::enum_name(static_cast<efsw::Error>(watchId)));
            } else {
                fileWatchId = watchId;
            }
        }
    }

    std::string Editor::getNameForNewLdrFile() {
        static uint32_t nameCounter = 0;
        std::string name;
        auto& fileRepo = ldr::file_repo::get();
        do {
            nameCounter++;
            name = fmt::format("Untitled{:d}.mpd", nameCounter);
        } while (fileRepo.hasFileCached(name));
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

    std::shared_ptr<etree::MpdNode>& Editor::getDocumentNode() {
        return documentNode;
    }

    bool Editor::isModified() const {
        return lastSavedVersion != documentNode->getVersion();
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
        if (lastSavedVersion != documentNode->getVersion()) {
            documentNode->writeChangesToLdrFile();
            ldr::writeFile(documentNode->ldrFile, filePath.value());
            lastSavedVersion = documentNode->getVersion();
        }
    }

    void Editor::saveAs(const std::filesystem::path& newPath) {
        filePath = newPath;
        ldr::file_repo::get().changeFileName(documentNode->ldrFile, newPath.filename().string());
        save();
    }

    void Editor::saveCopyAs(const std::filesystem::path& copyPath) {
        documentNode->writeChangesToLdrFile();
        ldr::writeFile(documentNode->ldrFile, copyPath);
    }

    void Editor::nodeSelectAddRemove(const std::shared_ptr<etree::Node>& node) {
        auto iterator = selectedNodes.find(node);
        node->selected = iterator == selectedNodes.end();
        if (node->selected) {
            selectedNodes.emplace(node, node->getVersion());
        } else {
            selectedNodes.erase(iterator);
        }
        updateSelectionVisualisation();
    }

    void Editor::nodeSelectSet(const std::shared_ptr<etree::Node>& node) {
        for (const auto& selectedNode: selectedNodes) {
            selectedNode.first->selected = false;
        }
        selectedNodes.clear();
        node->selected = true;
        selectedNodes.emplace(node, node->getVersion());
        updateSelectionVisualisation();
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
        updateSelectionVisualisation();
    }

    void Editor::nodeSelectAll() {
        nodeSelectNone();
        documentNode->selected = true;
        selectedNodes.emplace(documentNode, documentNode->getVersion());
        updateSelectionVisualisation();
    }

    void Editor::nodeSelectNone() {
        for (const auto& item: selectedNodes) {
            item.first->selected = false;
        }
        selectedNodes.clear();
        updateSelectionVisualisation();
    }

    void Editor::setStandard3dView(int i) {
        camera->setStandardView(i);
    }

    void Editor::rotateViewUp() { camera->mouseRotate(0, -1); }
    void Editor::rotateViewDown() { camera->mouseRotate(0, +1); }
    void Editor::rotateViewLeft() { camera->mouseRotate(-1, 0); }
    void Editor::rotateViewRight() { camera->mouseRotate(+1, 0); }
    void Editor::panViewUp() { camera->mousePan(0, -1); }
    void Editor::panViewDown() { camera->mousePan(0, +1); }
    void Editor::panViewLeft() { camera->mousePan(-1, 0); }
    void Editor::panViewRight() { camera->mousePan(+1, 0); }

    void Editor::insertLdrElement(const std::shared_ptr<ldr::File>& ldrFile) {
        switch (ldrFile->metaInfo.type) {
            case ldr::MPD_SUBFILE:
                documentNode->addSubfileInstanceNode(ldrFile, {1});
                documentNode->incrementVersion();
                break;
            case ldr::PART:
                documentNode->addChild(std::make_shared<etree::PartNode>(ldrFile, ldr::ColorReference{1}, documentNode, nullptr));
                documentNode->incrementVersion();
                break;
            case ldr::SUBPART:
            case ldr::PRIMITIVE:
            default: break;
        }
    }

    void Editor::deleteElement(const std::shared_ptr<etree::Node>& nodeToDelete) {
        auto parent = nodeToDelete->parent.lock();
        parent->removeChild(nodeToDelete);
        parent->incrementVersion();
        selectedNodes.erase(nodeToDelete);
        updateSelectionVisualisation();
    }

    void Editor::deleteSelectedElements() {
        for (const auto& item: selectedNodes) {
            deleteElement(item.first);
        }
        selectedNodes.clear();
        updateSelectionVisualisation();
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

    const std::string& Editor::getFilename() {
        return documentNode->ldrFile->metaInfo.name;
    }
    
    void Editor::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) {
        auto fullPath = std::filesystem::path(dir) / filename;
        if (filePath.has_value() && fullPath == filePath.value()) {
            spdlog::info(R"(editor file change detected: {} fullPath="{}" oldFilename="{}")", magic_enum::enum_name(action), fullPath.string(), oldFilename);
            rootNode->removeChild(documentNode);
            documentNode = std::make_shared<etree::MpdNode>(ldr::file_repo::get().reloadFile(filePath->string()), 1, rootNode);
            documentNode->createChildNodes();
            rootNode->addChild(documentNode);
            documentNode->incrementVersion();
        }
    }
    void Editor::updateSelectionVisualisation() {
        if (selectedNodes.empty()) {
            if (selectionVisualisationNode != nullptr && selectionVisualisationNode->visible) {
                selectionVisualisationNode->visible = false;
                selectionVisualisationNode->incrementVersion();
            }
        } else {
            if (selectionVisualisationNode == nullptr) {
                selectionVisualisationNode = std::make_shared<SelectionVisualisationNode>(rootNode);
                rootNode->addChild(selectionVisualisationNode);
            }
            selectionVisualisationNode->visible = false;

            if (selectedNodes.size() == 1) {
                const auto meshNode = std::dynamic_pointer_cast<etree::MeshNode>(selectedNodes.begin()->first);
                if (meshNode != nullptr) {
                    const auto relativeAABB = scene->getMeshCollection().getRelativeAABB(meshNode);
                    if (relativeAABB.isDefined()) {
                        const auto rotatedBBox = mesh::RotatedBoundingBox(relativeAABB, glm::vec3(0.f, 0.f, 0.f), glm::quat(1.f, 0.f, 0.f, 0.f)).transform(glm::transpose(meshNode->getAbsoluteTransformation()));
                        selectionVisualisationNode->visible = true;
                        selectionVisualisationNode->setRelativeTransformation(glm::transpose(rotatedBBox.getUnitBoxTransformation()));
                    }
                }
                selectedNodes.begin()->second = selectedNodes.begin()->first->getVersion();
            } else {
                mesh::AxisAlignedBoundingBox aabb;
                for (auto& node: selectedNodes) {
                    std::shared_ptr<etree::MeshNode> meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node.first);
                    if (meshNode != nullptr) {
                        aabb.addAABB(scene->getMeshCollection().getAbsoluteAABB(meshNode));
                    }
                    node.second = node.first->getVersion();
                }
                if (aabb.isDefined()) {
                    selectionVisualisationNode->visible = true;
                    glm::mat4 transf(1.f);
                    transf = glm::translate(transf, aabb.getCenter());
                    transf = glm::scale(transf, aabb.getSize() / 2.f);
                    spdlog::debug("center={}, size={}", aabb.getCenter(), aabb.getSize());
                    selectionVisualisationNode->setRelativeTransformation(glm::transpose(transf));
                } else {
                }
            }
            selectionVisualisationNode->incrementVersion();
        }
    }

    void Editor::update() {
        transformGizmo->update();
        for (const auto& item: selectedNodes) {
            if (item.first->getVersion() != item.second) {
                updateSelectionVisualisation();
                break;
            }
        }
    }

    SelectionVisualisationNode::SelectionVisualisationNode(const std::shared_ptr<Node>& parent) :
        MeshNode(1, parent, nullptr) {
        visibleInElementTree = false;
    }
    mesh_identifier_t SelectionVisualisationNode::getMeshIdentifier() const {
        return constants::MESH_ID_SELECTION_VISUALISATION;
    }
    void SelectionVisualisationNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
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
    bool SelectionVisualisationNode::isDisplayNameUserEditable() const {
        return false;
    }
}
