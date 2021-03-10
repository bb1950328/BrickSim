#include <glm/glm.hpp>
#include "constants.h"
#include <glm/gtc/matrix_transform.hpp>

#define XNUM(x) #x
#define NUM(x) XNUM(x)

namespace constants {
    const glm::mat4 LDU_TO_OPENGL_ROTATION = glm::rotate(glm::mat4(1.0f),//base
                                                         glm::radians(180.0f),//rotate 180° around
                                                         glm::vec3(1.0f, 0.0f, 0.0f));// x axis;
    const char* gitCommitHash = BRICKSIM_GIT_COMMIT_HASH;
    const char* versionString = NUM(BRICKSIM_VERSION_MAJOR) "." NUM(BRICKSIM_VERSION_MINOR) "." NUM(BRICKSIM_VERSION_PATCH);
}