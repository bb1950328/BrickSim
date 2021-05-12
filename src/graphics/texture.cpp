#include <mutex>
#include "texture.h"
#include "stb_image.h"
#include "../controller.h"
#include "../latest_log_messages_tank.h"


Texture::Texture(const std::filesystem::path &image) {
    unsigned char *data = stbi_load(image.string().c_str(), &width, &height, &nrChannels, 3);
    if (!data) {
        throw std::invalid_argument("texture not read successfully from file: " + image.string());
    }
    textureId = copyTextureToVram(width, height, nrChannels, data);
    stbi_image_free(data);
}

Texture::Texture(const unsigned char *fileData, unsigned int dataSize) {
    unsigned char *data = stbi_load_from_memory(fileData, dataSize, &width, &height, &nrChannels, 0);
    if (!data) {
        throw std::invalid_argument("texture not read successfully from memory: " + std::to_string((intptr_t) fileData));
    }
    textureId = copyTextureToVram(width, height, nrChannels, data);
    stbi_image_free(data);
}

Texture::~Texture() {
    controller::executeOpenGL([this](){
        glDeleteTextures(1, &textureId);
    });
}

unsigned int Texture::copyTextureToVram(int imgWidth, int imgHeight, int nrChannels, const unsigned char *data) {
    unsigned int textureId;
    controller::executeOpenGL([&](){
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        GLenum format;
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
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, format, imgWidth, imgHeight, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    });
    return textureId;
}

void Texture::bind(uint8_t slot) const {
    glActiveTexture(GL_TEXTURE0+slot);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

texture_id_t Texture::getID() const {
    return textureId;
}

glm::ivec2 Texture::getSize() const {
    return {width, height};
}
