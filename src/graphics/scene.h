#pragma once

#include "../element_tree.h"
#include "../helpers/ray.h"
#include "../types.h"
#include "camera.h"
#include "mesh/mesh_collection.h"
#include "overlay2d/element_collection.h"
#include <filesystem>
#include <map>

namespace bricksim::graphics {
    using framebuffer_size_t = unsigned short;

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
        constexpr scene_id_t THUMBNAIL_SCENE_ID = 10;
        constexpr scene_id_t ORIENTATION_CUBE_SCENE_ID = 20;
        constexpr scene_id_t FIRST_MAIN_SCENE_ID = 30;
        constexpr scene_id_t CONNECTION_VISUALIZATION_SCENE_ID = 40;
        std::shared_ptr<Scene> create(scene_id_t sceneId);
        std::shared_ptr<Scene>& get(scene_id_t sceneId);
        void remove(scene_id_t sceneId);
        uomap_t<scene_id_t, std::shared_ptr<Scene>>& getAll();
        void deleteAll();
    }

    class Scene {
    private:
        scene_id_t id;
        CompleteFramebuffer image{{64, 64}};
        std::optional<CompleteFramebuffer> selection;
        std::shared_ptr<etree::Node> rootNode;
        mesh::SceneMeshCollection meshCollection;
        overlay2d::ElementCollection overlayCollection;
        std::shared_ptr<Camera> camera;
        glm::usvec2 imageSize;
        glm::mat4 projectionMatrix;
        glm::mat4 currentImageViewMatrix;
        glm::mat4 currentSelectionImageViewMatrix;
        uint64_t imageEtreeVersion = 0;
        uint64_t selectionImageEtreeVersion = 0;
        bool drawTriangles = true;
        bool drawLines = true;
        bool currentImageDrawTriangles = true;
        bool currentImageDrawLines = true;
        glm::vec4 backgroundColor;

    public:
        explicit Scene(scene_id_t sceneId);//todo make constructor private and set scenes::create as a friend
        unsigned int getSelectionPixel(framebuffer_size_t x, framebuffer_size_t y);
        unsigned int getSelectionPixel(glm::usvec2 coords);
        void updateImage();
        [[nodiscard]] glm::vec3 worldToScreenCoordinates(glm::vec3 worldCoords) const;
        [[nodiscard]] Ray3 screenCoordinatesToWorldRay(glm::usvec2 screenCoords) const;

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
        const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(const color::RGB& rgbColor);
        void setBackgroundColor(const glm::vec4& newColor);
    };
}
