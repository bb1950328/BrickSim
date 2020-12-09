//
// Created by bb1950328 on 17.10.2020.
//

#ifndef BRICKSIM_CONSTANTS_H
#define BRICKSIM_CONSTANTS_H

#include "types.h"

namespace constants {
    constexpr float LDU_TO_OPENGL_SCALE = 0.01f;
    const glm::mat4 LDU_TO_OPENGL_ROTATION = glm::rotate(glm::mat4(1.0f),//base
                                                         glm::radians(180.0f),//rotate 180° around
                                                         glm::vec3(1.0f, 0.0f, 0.0f));// x axis;
    constexpr layer_t DEFAULT_LAYER=0;
}
#endif //BRICKSIM_CONSTANTS_H
