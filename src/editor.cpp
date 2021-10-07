#include "editor.h"
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "controller.h"
#include "ldr/file_repo.h"
#include "ldr/file_writer.h"
#include "spdlog/fmt/bundled/format.h"

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
        return lastSavedVersion != rootNode->getVersion();
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
            selectedNodes.insert(node);
        } else {
            selectedNodes.erase(iterator);
        }
    }

    void Editor::nodeSelectSet(const std::shared_ptr<etree::Node>& node) {
        for (const auto& selectedNode: selectedNodes) {
            selectedNode->selected = false;
        }
        selectedNodes.clear();
        node->selected = true;
        selectedNodes.insert(node);
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
                selectedNodes.insert(itNode);
            }
        }
    }

    void Editor::nodeSelectAll() {
        nodeSelectNone();
        documentNode->selected = true;
        selectedNodes.insert(documentNode);
    }

    void Editor::nodeSelectNone() {
        for (const auto& item: selectedNodes) {
            item->selected = false;
        }
        selectedNodes.clear();
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
    }

    void Editor::deleteSelectedElements() {
        for (const auto& item: selectedNodes) {
            deleteElement(item);
        }
    }

    void Editor::hideSelectedElements() {
        for (const auto& item: selectedNodes) {
            item->visible = false;
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
    const uoset_t<std::shared_ptr<etree::Node>>& Editor::getSelectedNodes() const {
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
}
