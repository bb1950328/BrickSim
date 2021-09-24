#pragma once

#include "../binary_file.h"
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

        static uomap_t<std::string, std::shared_ptr<Texture>> texturesFromBinaryFiles;

    public:
        explicit Texture(const std::filesystem::path& image);
        static std::shared_ptr<Texture> getFromBinaryFileCached(const std::shared_ptr<BinaryFile>& binaryFile);
        Texture(const unsigned char* fileData, unsigned int dataSize);
        Texture& operator=(const Texture&) = delete;
        Texture(const Texture&) = delete;
        ~Texture();

        void bind(uint8_t slot = 0) const;
        [[nodiscard]] texture_id_t getID() const;
        [[nodiscard]] glm::ivec2 getSize() const;
    };
}
