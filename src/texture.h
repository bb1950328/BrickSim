#ifndef BRICKSIM_TEXTURE_H
#define BRICKSIM_TEXTURE_H

#include <filesystem>
#include "glm/glm.hpp"
#include "types.h"

class Texture {
private:
    texture_id_t textureId;
    int width;
    int height;
    int nrChannels;

    static unsigned int copyTextureToVram(int imgWidth, int imgHeight, int nrChannels, const unsigned char *data);
public:
    explicit Texture(const std::filesystem::path &image);
    Texture(const unsigned char *fileData, unsigned int dataSize);
    Texture & operator=(const Texture&) = delete;
    Texture(const Texture&) = delete;
    ~Texture();

    void bind(uint8_t slot=0) const;
    [[nodiscard]] texture_id_t getID() const;
    [[nodiscard]] glm::ivec2 getSize() const;
};

#endif //BRICKSIM_TEXTURE_H
