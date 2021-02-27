

#ifndef BRICKSIM_SCENE_H
#define BRICKSIM_SCENE_H

#include <filesystem>
#include <optional>
#include "shaders/shader.h"
#include "element_tree.h"

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
    CompleteFramebuffer image;
    std::optional<CompleteFramebuffer> selection;
    std::shared_ptr<etree::Node> rootNode;

    static std::unique_ptr<Shader> triangleShader;
    static std::unique_ptr<Shader> lineShader;
    static std::unique_ptr<Shader> optionalLineShader;
    static std::unique_ptr<Shader> textureShader;
    static std::unique_ptr<Shader> overlayShader;
public:
    Scene();
};

#endif //BRICKSIM_SCENE_H
