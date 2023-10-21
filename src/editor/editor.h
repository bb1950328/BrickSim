#pragma once

#include "../connection/engine.h"
#include "../element_tree.h"
#include "../graphics/scene.h"
#include "../gui/graphical_transform/provider.h"
#include "../ldr/file_repo.h"
#include "efsw/efsw.hpp"

namespace bricksim {

    //todo move this to separate file
    class SelectionVisualizationNode : public etree::MeshNode {
    public:
        explicit SelectionVisualizationNode(const std::shared_ptr<Node>& parent);

        mesh_identifier_t getMeshIdentifier() const override;

        void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed,
                       const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;

        [[nodiscard]] bool isDisplayNameUserEditable() const override;
    };

    class Editor : private efsw::FileWatchListener, public std::enable_shared_from_this<Editor> {
    public:
        static std::shared_ptr<Editor> createNew();
        static std::shared_ptr<Editor> openFile(const std::filesystem::path& path);

        explicit Editor();
        explicit Editor(const std::filesystem::path& path);
        Editor(const Editor& other) = delete;
        Editor& operator=(const Editor& other) = delete;
        Editor(Editor&& other) noexcept = delete;
        Editor& operator=(Editor&& other) = delete;
        ~Editor() override;

        const std::optional<std::filesystem::path>& getFilePath();
        std::shared_ptr<etree::RootNode>& getRootNode();
        std::shared_ptr<etree::ModelNode>& getEditingModel();
        void setEditingModel(const std::shared_ptr<etree::ModelNode>& newEditingModel, bool saveInHistory = true);
        void editLastModelInHistory();
        std::shared_ptr<ldr::FileNamespace>& getFileNamespace();
        [[nodiscard]] bool isModified() const;
        std::shared_ptr<graphics::Scene>& getScene();
        std::unique_ptr<graphical_transform::BaseAction>& getCurrentTransformAction();
        [[nodiscard]] const std::shared_ptr<graphics::CadCamera>& getCamera() const;
        [[nodiscard]] connection::Engine& getConnectionEngine();
        [[nodiscard]] const uomap_t<std::shared_ptr<etree::Node>, uint64_t>& getSelectedNodes() const;

        [[nodiscard]] std::string getFilename() const;
        [[nodiscard]] std::string getDisplayName() const;

        void save();
        void saveAs(const std::filesystem::path& newPath);
        void saveCopyAs(const std::filesystem::path& copyPath);

        void nodeSelectAddRemove(const std::shared_ptr<etree::Node>& node);
        void nodeSelectUntil(const std::shared_ptr<etree::Node>& node);
        void nodeSelectSet(const std::shared_ptr<etree::Node>& node);
        void nodeSelectSet(const uoset_t<std::shared_ptr<etree::Node>>& nodes);
        void nodeSelectAll();
        void nodeSelectNone();
        void nodeSelectConnected();

        void openContextMenuNodeSelectedOrClicked(const std::shared_ptr<etree::Node>& clickedNode);
        void openContextMenuNoNode();

        void startTransformingSelectedNodes(graphical_transform::GraphicalTransformationType type);
        void endNodeTransformation();
        void cancelNodeTransformation();
        void updateCursorPos(const std::optional<glm::svec2>& value);

        void setStandard3dView(int i);//todo refactor this into enum
        void rotateViewUp();
        void rotateViewDown();
        void rotateViewLeft();
        void rotateViewRight();
        void panViewUp();
        void panViewDown();
        void panViewLeft();
        void panViewRight();
        void centerElementIn3dView(const std::shared_ptr<etree::Node>& node);

        void insertLdrElement(const std::shared_ptr<ldr::File>& ldrFile);
        void deleteElement(const std::shared_ptr<etree::Node>& nodeToDelete);

        void deleteSelectedElements();
        void hideSelectedElements();
        void unhideAllElements();

        void inlineElement(const std::shared_ptr<etree::Node>& nodeToInline);
        void inlineElement(const std::shared_ptr<etree::Node>& nodeToInline, bool updateSelectionVisualization);
        void inlineSelectedElements();

        void update();

        bool isActive() const;

    private:
        void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
                              efsw::Action action, std::string oldFilename) override;

        void updateSelectionVisualization();

        static std::string getNameForNewLdrFile();
        void init(const std::shared_ptr<ldr::File>& ldrFile);

        std::optional<std::filesystem::path> filePath;
        std::optional<efsw::WatchID> fileWatchId;
        bool enableFileAutoReload = true;
        std::shared_ptr<etree::RootNode> rootNode;
        std::shared_ptr<etree::ModelNode> editingModel;
        std::deque<std::weak_ptr<etree::ModelNode>> editingModelHistory;
        std::shared_ptr<ldr::FileNamespace> fileNamespace;
        scene_id_t sceneId{};
        std::shared_ptr<graphics::Scene> scene;
        std::unique_ptr<graphical_transform::BaseAction> currentTransformAction = nullptr;
        uomap_t<std::shared_ptr<etree::ModelNode>, etree::Node::version_t> lastSavedVersions;
        ///value is last version, use to check if selected node was modified in the meantime
        uomap_t<std::shared_ptr<etree::Node>, uint64_t> selectedNodes;
        std::shared_ptr<SelectionVisualizationNode> selectionVisualizationNode;
        std::shared_ptr<graphics::CadCamera> camera;
        connection::Engine connectionEngine;
        ///empty means cursor is outside window
        std::optional<glm::svec2> cursorPos;

        void addConnectorDataVisualization(const std::shared_ptr<etree::Node>& node) const;
        [[nodiscard]] bool isModified(const std::shared_ptr<etree::ModelNode>& model) const;
        void setAsActiveEditor();
        void deleteModelInstances(const std::shared_ptr<etree::ModelNode>& modelToDelete, const std::shared_ptr<etree::Node>& currentNode);
        void writeTo(const std::filesystem::path& mainFilePath);
    };
}
