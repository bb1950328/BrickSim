

#ifndef BRICKSIM_SCENE_H
#define BRICKSIM_SCENE_H

#include <filesystem>
#include <optional>
#include "shaders/shader.h"
#include "element_tree.h"
#include "mesh_collection.h"

typedef unsigned short framebuffer_size_t;

class CompleteFramebuffer {
private:
    unsigned int framebuffer;
    unsigned int textureColorbuffer;
    unsigned int renderBufferObject;
    framebuffer_size_t currentSize[2];
public:
    CompleteFramebuffer(framebuffer_size_t width, framebuffer_size_t height);
    CompleteFramebuffer(const CompleteFramebuffer&) = delete;
    CompleteFramebuffer& operator=(const CompleteFramebuffer&) = delete;
    virtual ~CompleteFramebuffer();
    void saveImage(const std::filesystem::path& path);
    void resize(framebuffer_size_t newWidth, framebuffer_size_t newHeight);
};

class Scene {
private:
    scene_id_t id;
    CompleteFramebuffer image;
    std::optional<CompleteFramebuffer> selection;
    std::shared_ptr<etree::Node> rootNode;
    SceneMeshCollection meshCollection;

    static std::unique_ptr<Shader> triangleShader;
    static std::unique_ptr<Shader> lineShader;
    static std::unique_ptr<Shader> optionalLineShader;
    static std::unique_ptr<Shader> textureShader;
    static std::unique_ptr<Shader> overlayShader;
public:
    explicit Scene(scene_id_t sceneId);
};

namespace scenes {
    std::weak_ptr<Scene> createScene(scene_id_t sceneId);
}

#endif //BRICKSIM_SCENE_H
