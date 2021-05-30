set_source_files_properties(constants.cpp
        TARGET_DIRECTORY BrickSimLib
        PROPERTIES COMPILE_DEFINITIONS "BRICKSIM_TOTAL_HOURS=${TOTAL_HOURS};BRICKSIM_GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT};BRICKSIM_GIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\";BRICKSIM_VERSION_MAJOR=${BrickSim_VERSION_MAJOR};BRICKSIM_VERSION_MINOR=${BrickSim_VERSION_MINOR};BRICKSIM_VERSION_PATCH=${BrickSim_VERSION_PATCH};")
target_sources(BrickSimLib PRIVATE
        constants.cpp
        constants.h
        resources.cpp
        resources.h
        )