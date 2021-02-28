

#ifndef BRICKSIM_TYPES_H
#define BRICKSIM_TYPES_H

typedef unsigned char layer_t;
typedef unsigned char scene_id_t;
typedef unsigned int element_id_t;
typedef unsigned char color_component_t;

namespace glm {
    typedef vec<1, unsigned short, defaultp>	usvec1;
    typedef vec<2, unsigned short, defaultp>	usvec2;
    typedef vec<3, unsigned short, defaultp>	usvec3;
    typedef vec<4, unsigned short, defaultp>	usvec4;
}

#endif //BRICKSIM_TYPES_H
