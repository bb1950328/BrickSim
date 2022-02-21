#include "constants.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define XNUM(x) #x
#define NUM(x) XNUM(x)

namespace bricksim::constants {
    const glm::mat4 LDU_TO_OPENGL_ROTATION = glm::rotate(glm::mat4(1.0f),             //base
                                                         glm::radians(180.0f),        //rotate 180° around
                                                         glm::vec3(1.0f, 0.0f, 0.0f));// x axis;
    const glm::mat4 LDU_TO_OPENGL = glm::scale(LDU_TO_OPENGL_ROTATION, glm::vec3(LDU_TO_OPENGL_SCALE));
    const glm::mat4 OPENGL_TO_LDU = glm::inverse(LDU_TO_OPENGL);

    const char* const SEGMENT_PART_NAMES = "14301k01.dat;14301k02.dat;166.dat;16981k03.dat;16981k04.dat;23221k01.dat;23221k02.dat;2700.dat;27328k01.dat;27328k02.dat;27965k01.dat;27965k02.dat;30191k02.dat;32216.dat;32216.dat;32216.dat;32216.dat;32216.dat;32216.dat;3229ac01.dat;3229bc01.dat;3241ac01.dat;343.dat;43898.dat;43898ps1.dat;43903k01.dat;43903k02.dat;53992k01.dat;53992k02.dat;572b.dat;57539k01.dat;57539k02.dat;64022c01.dat;64022c02.dat;64022.dat;64572.dat;64573.dat;66821k01.dat;66821k02.dat;66821k03.dat;66821k04.dat;66821k05.dat;680.dat;681.dat;682.dat;71372c01.dat;71533k03.dat;71735k01.dat;754.dat;755.dat;756.dat;758.dat;759.dat;77.dat;79.dat;80.dat;866c01.dat;88492.dat;88493.dat;92713k01.dat;92713k02.dat;932.dat;u9028.dat;u9190.dat;u9218.dat;u9392.dat";

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
    extern const char* versionString;

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

    const char* gitCommitHash =
#ifdef BRICKSIM_GIT_COMMIT_HASH
            BRICKSIM_GIT_COMMIT_HASH;
#else
    #warning Please set the BRICKSIM_GIT_COMMIT_HASH macro
            "<unknown>";
#endif
    ;
    const char* versionString = NUM(BRICKSIM_VERSION_MAJOR) "." NUM(BRICKSIM_VERSION_MINOR) "." NUM(BRICKSIM_VERSION_PATCH);

    const char* LDRAW_LIBRARY_DOWNLOAD_URL = "https://www.ldraw.org/library/updates/complete.zip";
}