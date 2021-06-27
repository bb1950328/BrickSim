#ifndef BRICKSIM_TYPES_H
#define BRICKSIM_TYPES_H

#include <glm/fwd.hpp>
namespace bricksim {
    typedef unsigned char layer_t;
    typedef unsigned char scene_id_t;
    typedef unsigned int element_id_t;
    typedef unsigned char color_component_t;
    typedef unsigned int texture_id_t;
    typedef uint64_t mesh_identifier_t;
}
namespace glm {
    typedef vec<1, unsigned short, defaultp>	usvec1;
    typedef vec<2, unsigned short, defaultp>	usvec2;
    typedef vec<3, unsigned short, defaultp>	usvec3;
    typedef vec<4, unsigned short, defaultp>	usvec4;
    typedef vec<1, signed short, defaultp>	    svec1;
    typedef vec<2, signed short, defaultp>	    svec2;
    typedef vec<3, signed short, defaultp>	    svec3;
    typedef vec<4, signed short, defaultp>	    svec4;
}

#endif //BRICKSIM_TYPES_H
