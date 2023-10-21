#include "texture.h"
#include "../controller.h"
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <stb_image_write.h>

namespace bricksim::graphics {

    uomap_t<std::string, std::shared_ptr<Texture>> Texture::texturesFromBinaryFiles;

    Texture::Texture(const std::filesystem::path& image) {
        unsigned char* data = stbi_load(image.string().c_str(), &width, &height, &nrChannels, 0);
        if (!data) {
            throw std::invalid_argument("texture not read successfully from file: " + image.string());
        }
        textureId = copyTextureToVram(width, height, nrChannels, data);
        stbi_image_free(data);
    }

    Texture::Texture(const unsigned char* fileData, unsigned int dataSize) {
        unsigned char* data = stbi_load_from_memory(fileData, dataSize, &width, &height, &nrChannels, 0);
        if (!data) {
            throw std::invalid_argument("texture not read successfully from memory: " + std::to_string((intptr_t)fileData));
        }
        textureId = copyTextureToVram(width, height, nrChannels, data);
        stbi_image_free(data);
    }

    Texture::~Texture() {
        controller::executeOpenGL([this]() {
            glDeleteTextures(1, &textureId);
        });
    }

    unsigned int Texture::copyTextureToVram(int imgWidth, int imgHeight, int nrChannels, const unsigned char* data) {
        unsigned int textureId;
        controller::executeOpenGL([&textureId, &nrChannels, &imgWidth, &imgHeight, &data]() {
            int maxTextureSize;
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
            if (maxTextureSize < imgWidth || maxTextureSize < imgHeight) {
                throw std::invalid_argument(fmt::format("GPU max texture size is {}x{}, but image is {}x{}", maxTextureSize, maxTextureSize, imgWidth, imgHeight));
            }

            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);

            GLint format = getGlFormatFromNrChannels(nrChannels);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, format, imgWidth, imgHeight, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        });
        return textureId;
    }
    GLint Texture::getGlFormatFromNrChannels(int nrChannels) {
        GLint format;
        if (nrChannels == 1) {
            format = GL_RED;
        } else if (nrChannels == 3) {
            format = GL_RGB;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
        } else {
            spdlog::warn("image has a weird number of channels: {}", nrChannels);
            format = GL_RGB;
        }
        return format;
    }

    void Texture::bind(uint8_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }

    texture_id_t Texture::getID() const {
        return textureId;
    }

    glm::ivec2 Texture::getSize() const {
        return {width, height};
    }

    std::shared_ptr<Texture> Texture::getFromBinaryFileCached(const std::shared_ptr<BinaryFile>& binaryFile) {
        const auto it = texturesFromBinaryFiles.find(binaryFile->name);
        if (it != texturesFromBinaryFiles.end()) {
            return it->second;
        } else {
            return texturesFromBinaryFiles.emplace(binaryFile->name, std::make_shared<Texture>(&binaryFile->data[0], binaryFile->data.size())).first->second;
        }
    }
    void Texture::deleteCached() {
        texturesFromBinaryFiles.clear();
    }

    void Texture::unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    Texture::Texture(texture_id_t textureId, int width, int height, int nrChannels) :
        textureId(textureId), width(width), height(height), nrChannels(nrChannels) {}

    void Texture::saveToFile(const std::filesystem::path& path) {
        auto data = std::vector<GLubyte>();
        data.resize((size_t)width * height * nrChannels);
        controller::executeOpenGL([this, &data]() {
            GLint format = getGlFormatFromNrChannels(nrChannels);
            bind();
            glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, &data[0]);
            unbind();
        });

        const bool success = util::writeImage(path.string().c_str(), &data[0], width, height, nrChannels);
        if (!success) {
            throw std::invalid_argument("Texture::saveToFile did not succeed");
        }
    }
}
