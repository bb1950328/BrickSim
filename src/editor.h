#pragma once

#include "element_tree.h"
#include "graphics/scene.h"
#include "ldr/file_repo.h"
#include "transform_gizmo.h"
#include <efsw/efsw.hpp>

namespace bricksim {

    class SelectionVisualisationNode : public etree::MeshNode {
    public:
        SelectionVisualisationNode(const std::shared_ptr<Node>& parent);
        virtual mesh_identifier_t getMeshIdentifier() const override;
        virtual void addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) override;
        [[nodiscard]] virtual bool isDisplayNameUserEditable() const override;
    };

    class Editor : efsw::FileWatchListener {
    public:
        static std::shared_ptr<Editor> createNew();
        static std::shared_ptr<Editor> openFile(const std::filesystem::path& path);

        explicit Editor();
        explicit Editor(const std::filesystem::path& path);
        Editor(const Editor& other) = delete;
        Editor& operator=(const Editor& other) = delete;
        Editor(Editor&& other) noexcept = default;
        Editor& operator=(Editor&& other) = default;
        ~Editor() override;

        const std::optional<std::filesystem::path>& getFilePath();
        std::shared_ptr<etree::RootNode>& getRootNode();
        std::shared_ptr<etree::MpdNode>& getDocumentNode();
        [[nodiscard]] bool isModified() const;
        std::shared_ptr<graphics::Scene>& getScene();
        std::unique_ptr<transform_gizmo::TransformGizmo>& getTransformGizmo();
        [[nodiscard]] const std::shared_ptr<graphics::CadCamera>& getCamera() const;
        [[nodiscard]] const uoset_t<std::shared_ptr<etree::Node>>& getSelectedNodes() const;

        const std::string& getFilename();

        void save();
        void saveAs(const std::filesystem::path& newPath);
        void saveCopyAs(const std::filesystem::path& copyPath);

        void nodeSelectAddRemove(const std::shared_ptr<etree::Node>& node);
        void nodeSelectUntil(const std::shared_ptr<etree::Node>& node);
        void nodeSelectSet(const std::shared_ptr<etree::Node>& node);
        void nodeSelectAll();
        void nodeSelectNone();

        bool isNodeClickable(const std::shared_ptr<etree::Node>& node);
        void nodeClicked(const std::shared_ptr<etree::Node>& clickedNode, bool ctrlPressed, bool shiftPressed);

        bool isNodeDraggable(const std::shared_ptr<etree::Node>& node);
        void startNodeDrag(std::shared_ptr<etree::Node>& draggedNode, const glm::svec2& initialCursorPos);
        void updateNodeDragDelta(glm::usvec2 delta);
        void endNodeDrag();

        void setStandard3dView(int i);//todo refactor this into enum
        void rotateViewUp();
        void rotateViewDown();
        void rotateViewLeft();
        void rotateViewRight();
        void panViewUp();
        void panViewDown();
        void panViewLeft();
        void panViewRight();

        void insertLdrElement(const std::shared_ptr<ldr::File>& ldrFile);
        void deleteElement(const std::shared_ptr<etree::Node>& nodeToDelete);

        void deleteSelectedElements();
        void hideSelectedElements();
        void unhideAllElements();

    private:
        void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) override;
        void updateSelectionVisualisation();
        static std::string getNameForNewLdrFile();
        void init(const std::shared_ptr<ldr::File>& ldrFile);

        std::optional<std::filesystem::path> filePath;
        std::optional<efsw::WatchID> fileWatchId;
        std::shared_ptr<etree::RootNode> rootNode;
        std::shared_ptr<etree::MpdNode> documentNode;
        scene_id_t sceneId{};
        std::shared_ptr<graphics::Scene> scene;
        std::unique_ptr<transform_gizmo::TransformGizmo> transformGizmo;
        uint64_t lastSavedVersion = 0;
        uoset_t<std::shared_ptr<etree::Node>> selectedNodes;
        std::shared_ptr<SelectionVisualisationNode> selectionVisualisationNode;
        std::shared_ptr<graphics::CadCamera> camera;

        enum class DraggingNodeType {
            NONE,
            TRANSFORM_GIZMO,
        };
        DraggingNodeType currentlyDraggingNodeType = DraggingNodeType::NONE;//todo change this to object oriented design
    };
}