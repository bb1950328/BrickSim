#pragma once

#include "../types.h"
#include <filesystem>
#include <glm/glm.hpp>
namespace bricksim::graphics {
    class Texture {
    private:
        texture_id_t textureId;
        int width;
        int height;
        int nrChannels;

        static unsigned int copyTextureToVram(int imgWidth, int imgHeight, int nrChannels, const unsigned char* data);

    public:
        explicit Texture(const std::filesystem::path& image);
        Texture(const unsigned char* fileData, unsigned int dataSize);
        Texture& operator=(const Texture&) = delete;
        Texture(const Texture&) = delete;
        ~Texture();

        void bind(uint8_t slot = 0) const;
        [[nodiscard]] texture_id_t getID() const;
        [[nodiscard]] glm::ivec2 getSize() const;
    };
}
