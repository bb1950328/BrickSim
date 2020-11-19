//
// Created by Bader on 18.11.2020.
//

#ifndef BRICKSIM_ORIENTATION_CUBE_H
#define BRICKSIM_ORIENTATION_CUBE_H
#include "glm/glm.hpp"
#include "renderer.h"

struct TexturedVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
};

namespace orientation_cube {
    unsigned int getImage();
};

#endif //BRICKSIM_ORIENTATION_CUBE_H
