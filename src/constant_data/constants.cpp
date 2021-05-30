#include "glm/glm.hpp"
#include "constants.h"
#include "glm/gtc/matrix_transform.hpp"

#define XNUM(x) #x
#define NUM(x) XNUM(x)

namespace constants {
    const glm::mat4 LDU_TO_OPENGL_ROTATION = glm::rotate(glm::mat4(1.0f),//base
                                                         glm::radians(180.0f),//rotate 180° around
                                                         glm::vec3(1.0f, 0.0f, 0.0f));// x axis;
    const glm::mat4 LDU_TO_OPENGL = glm::scale(constants::LDU_TO_OPENGL_ROTATION, glm::vec3(constants::LDU_TO_OPENGL_SCALE));
    const glm::mat4 OPENGL_TO_LDU = glm::inverse(LDU_TO_OPENGL);

    const uint16_t versionMajor =
#ifdef BRICKSIM_VERSION_MAJOR
            BRICKSIM_VERSION_MAJOR;
#else
#warning Please set the BRICKSIM_VERSION_MAJOR macro
            0;
#endif
    const uint16_t versionMinor =
#ifdef BRICKSIM_VERSION_MINOR
            BRICKSIM_VERSION_MINOR;
#else
#warning Please set the BRICKSIM_VERSION_MINOR macro
            0;
#endif
    ;
    const uint16_t versionPatch =
#ifdef BRICKSIM_VERSION_PATCH
            BRICKSIM_VERSION_PATCH;
#else
#warning Please set the BRICKSIM_VERSION_PATCH macro
            0;
#endif
    ;
    extern const char *versionString;

    const float totalWorkHours =
#ifdef BRICKSIM_TOTAL_HOURS
            BRICKSIM_TOTAL_HOURS;
#else
#warning Please set the BRICKSIM_TOTAL_HOURS macro
            0;
#endif
    ;
    const uint16_t gitCommitCount =
#ifdef BRICKSIM_GIT_COMMIT_COUNT
            BRICKSIM_GIT_COMMIT_COUNT;
#else
#warning Please set the BRICKSIM_GIT_COMMIT_COUNT macro
            0;
#endif
    ;

    const char *gitCommitHash =
#ifdef BRICKSIM_GIT_COMMIT_HASH
            BRICKSIM_GIT_COMMIT_HASH;
#else
    #warning Please set the BRICKSIM_GIT_COMMIT_HASH macro
    "<unknown>";
#endif
    ;
    const char *versionString = NUM(BRICKSIM_VERSION_MAJOR) "." NUM(BRICKSIM_VERSION_MINOR) "." NUM(BRICKSIM_VERSION_PATCH);
}