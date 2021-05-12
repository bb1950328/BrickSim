#ifndef BRICKSIM_SCENE_H
#define BRICKSIM_SCENE_H

#include <filesystem>
#include <optional>
#include "../shaders/shaders.h"
#include "../element_tree.h"
#include "mesh_collection.h"
#include "camera.h"
#include "overlay_2d.h"

typedef unsigned short framebuffer_size_t;

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
    [[nodiscard]] const glm::usvec2 &getSize() const;
    void setSize(const glm::usvec2 &newSize);
};

class Scene;

namespace scenes {
    constexpr scene_id_t MAIN_SCENE_ID = 0;
    constexpr scene_id_t THUMBNAIL_SCENE_ID = 10;
    constexpr scene_id_t ORIENTATION_CUBE_SCENE_ID = 20;
    std::shared_ptr<Scene> create(scene_id_t sceneId);
    std::shared_ptr<Scene> get(scene_id_t sceneId);
    std::map<scene_id_t, std::shared_ptr<Scene>>& getAll();
    void deleteAll();
}

class Scene {
private:
    scene_id_t id;
    CompleteFramebuffer image;
    std::optional<CompleteFramebuffer> selection;
    std::shared_ptr<etree::Node> rootNode;
    SceneMeshCollection meshCollection;
    overlay2d::ElementCollection overlayCollection;
    std::shared_ptr<Camera> camera;
    glm::usvec2 imageSize;
    glm::mat4 projectionMatrix;
    glm::mat4 currentImageViewMatrix, currentSelectionImageViewMatrix;
    bool imageUpToDate, selectionImageUpToDate, elementTreeRereadNeeded;
    void rereadElementTreeIfNeeded();
public:
    explicit Scene(scene_id_t sceneId);//todo make constructor private and set scenes::create as a friend
    unsigned int getSelectionPixel(framebuffer_size_t x, framebuffer_size_t y);
    void updateImage();
    void elementTreeChanged();
    glm::usvec2 worldToScreenCoordinates(glm::vec3 worldCoords);

    [[nodiscard]] const std::shared_ptr<etree::Node> &getRootNode() const;
    void setRootNode(const std::shared_ptr<etree::Node> &newRootNode);
    [[nodiscard]] const std::shared_ptr<Camera> &getCamera() const;
    void setCamera(const std::shared_ptr<Camera> &newCamera);
    [[nodiscard]] const glm::usvec2 &getImageSize() const;
    void setImageSize(const glm::usvec2 &newImageSize);
    [[nodiscard]] const CompleteFramebuffer &getImage() const;
    [[nodiscard]] const std::optional<CompleteFramebuffer> & getSelectionImage() const;
    [[nodiscard]] const SceneMeshCollection &getMeshCollection() const;
    [[nodiscard]] overlay2d::ElementCollection & getOverlayCollection();
};

#endif //BRICKSIM_SCENE_H
