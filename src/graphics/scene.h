#pragma once

#include "../element_tree.h"
#include "../helpers/ray.h"
#include "../types.h"
#include "camera.h"
#include "mesh/mesh_collection.h"
#include "overlay_2d.h"
#include <filesystem>
#include <map>

typedef unsigned short framebuffer_size_t;
namespace bricksim::graphics {
    class CompleteFramebuffer {
    private:
        unsigned int framebuffer;
        unsigned int textureColorbuffer;
        unsigned int renderBufferObject;
        glm::usvec2 size;
        void deallocate() const;
        void allocate();

    public:
        explicit CompleteFramebuffer(glm::usvec2 size_);
        CompleteFramebuffer(const CompleteFramebuffer&) = delete;
        CompleteFramebuffer& operator=(const CompleteFramebuffer&) = delete;
        virtual ~CompleteFramebuffer();
        void saveImage(const std::filesystem::path& path) const;
        [[nodiscard]] unsigned int getFBO() const;
        [[nodiscard]] unsigned int getTexBO() const;
        [[nodiscard]] unsigned int getRBO() const;
        [[nodiscard]] const glm::usvec2& getSize() const;
        void setSize(const glm::usvec2& newSize);
    };

    class Scene;

    namespace scenes {
        constexpr scene_id_t MAIN_SCENE_ID = 0;
        constexpr scene_id_t THUMBNAIL_SCENE_ID = 10;
        constexpr scene_id_t ORIENTATION_CUBE_SCENE_ID = 20;
        std::shared_ptr<Scene> create(scene_id_t sceneId);
        std::shared_ptr<Scene> get(scene_id_t sceneId);
        uomap_t<scene_id_t, std::shared_ptr<Scene>>& getAll();
        void deleteAll();
    }

    class Scene {
    private:
        scene_id_t id;
        CompleteFramebuffer image;
        std::optional<CompleteFramebuffer> selection;
        std::shared_ptr<etree::Node> rootNode;
        mesh::SceneMeshCollection meshCollection;
        overlay2d::ElementCollection overlayCollection;
        std::shared_ptr<Camera> camera;
        glm::usvec2 imageSize;
        glm::mat4 projectionMatrix;
        glm::mat4 currentImageViewMatrix, currentSelectionImageViewMatrix;
        bool imageUpToDate, selectionImageUpToDate, elementTreeRereadNeeded;
        bool drawTriangles = true, drawLines = true;
        bool currentImageDrawTriangles = true, currentImageDrawLines = true;
        void rereadElementTreeIfNeeded();

    public:
        explicit Scene(scene_id_t sceneId);//todo make constructor private and set scenes::create as a friend
        unsigned int getSelectionPixel(framebuffer_size_t x, framebuffer_size_t y);
        unsigned int getSelectionPixel(glm::usvec2 coords);
        void updateImage();
        void elementTreeChanged();
        glm::usvec2 worldToScreenCoordinates(glm::vec3 worldCoords);
        Ray3 screenCoordinatesToWorldRay(glm::usvec2 screenCoords);

        [[nodiscard]] const std::shared_ptr<etree::Node>& getRootNode() const;
        void setRootNode(const std::shared_ptr<etree::Node>& newRootNode);
        [[nodiscard]] const std::shared_ptr<Camera>& getCamera() const;
        void setCamera(const std::shared_ptr<Camera>& newCamera);
        [[nodiscard]] const glm::usvec2& getImageSize() const;
        void setImageSize(const glm::usvec2& newImageSize);
        [[nodiscard]] const CompleteFramebuffer& getImage() const;
        [[nodiscard]] const std::optional<CompleteFramebuffer>& getSelectionImage() const;
        [[nodiscard]] const mesh::SceneMeshCollection& getMeshCollection() const;
        [[nodiscard]] overlay2d::ElementCollection& getOverlayCollection();
        [[nodiscard]] bool* isDrawTriangles();
        [[nodiscard]] bool* isDrawLines();
    };
}
