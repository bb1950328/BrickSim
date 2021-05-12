cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

foreach(SDKPATH IN LISTS /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
    if (EXISTS "${SDKPATH}")
        set(CMAKE_OSX_SYSROOT ${SDKPATH})
    endif()
endforeach()
project(BrickSim)

#file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)#prevent profiles from loading in parallel, causes problems with submodules

add_library(BrickSimLib STATIC)

add_executable(BrickSim src/main.cpp)
target_link_libraries(BrickSim PRIVATE BrickSimLib)
add_dependencies(BrickSim BrickSimLib)

add_executable(BrickSimTests src/test/catchMain.cpp)
target_link_libraries(BrickSimTests PRIVATE Catch2::Catch2)
add_dependencies(BrickSimTests BrickSimLib)
target_link_libraries(BrickSimTests PRIVATE BrickSimLib)
target_compile_definitions(BrickSimTests PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

#target_link_libraries(BrickSimLib PUBLIC -static)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
target_include_directories(BrickSimLib PRIVATE ${OPENGL_INCLUDE_DIRS})
target_link_libraries(BrickSimLib PRIVATE ${OPENGL_LIBRARIES})
if (UNIX)
    if (APPLE)
        target_link_libraries(BrickSimLib PRIVATE "-framework Cocoa")
        target_link_libraries(BrickSimLib PRIVATE "-framework IOKit")
        target_link_libraries(BrickSimLib PRIVATE "-framework CoreVideo")
    endif ()
endif ()

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(src/lib/glfw)
target_include_directories(BrickSimLib PUBLIC src/lib/glfw/include)
target_link_libraries(BrickSimLib PRIVATE glfw)
target_link_libraries(BrickSimLib PRIVATE ${GLFW_LIBRARIES})
if (UNIX)# todo check if this is necessary
    if (NOT APPLE)
        find_package(X11 REQUIRED)
        target_link_libraries(BrickSimLib PRIVATE ${X11_LIBRARIES})
    endif ()
endif ()
target_compile_definitions(BrickSimLib PRIVATE GLFW_INCLUDE_NONE)

# GLUT
find_package(GLUT REQUIRED)
target_include_directories(BrickSimLib PRIVATE ${GLUT_INCLUDE_DIRS})
target_link_libraries(BrickSimLib PRIVATE ${GLUT_LIBRARY})

# threads
if (UNIX)
    if (NOT APPLE)
        find_package(Threads REQUIRED)
        target_link_libraries(BrickSimLib PRIVATE Threads::Threads)
    endif ()
endif ()

# libzip
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)# https://github.com/nih-at/libzip/issues/66
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(ENABLE_BZIP2 OFF)
set(ENABLE_LZMA OFF)
set(ENABLE_ZSTD OFF)
add_subdirectory(src/lib/libzip)
target_link_libraries(BrickSimLib PUBLIC libzip::zip)

# SqliteCpp
set(SQLITECPP_USE_STACK_PROTECTION OFF)
add_subdirectory(src/lib/SQLiteCpp)
target_include_directories(BrickSimLib PUBLIC src/lib/SQLiteCpp/include)
target_link_libraries(BrickSimLib PRIVATE SQLiteCpp)
target_link_libraries(BrickSimLib PRIVATE sqlite3)

# cURL
if(APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
    find_package(openssl REQUIRED)
    message(STATUS "openssl found in main CMakeLists.txt")
endif()
set(HTTP_ONLY ON)
set(CMAKE_USE_LIBSSH2 OFF)
add_subdirectory(src/lib/curl)
target_link_libraries(BrickSimLib PUBLIC CURL::libcurl)

# spdlog
add_subdirectory(src/lib/spdlog)
target_include_directories(BrickSimLib PUBLIC spdlog::spdlog)
target_link_libraries(BrickSimLib PUBLIC spdlog::spdlog)

#Catch2
add_subdirectory(src/lib/Catch2)

#GLM
add_subdirectory(src/lib/glm)
target_link_libraries(BrickSimLib PUBLIC glm::glm)

#tinyfiledialogs
target_include_directories(BrickSimLib PUBLIC src/lib/tinyfiledialogs)
target_sources(BrickSimLib PRIVATE src/lib/tinyfiledialogs/tinyfiledialogs.c)

#rapidjson
target_include_directories(BrickSimLib PUBLIC src/lib/rapidjson/include)

#stb
target_include_directories(BrickSimLib PUBLIC src/lib/stb)

add_subdirectory(src)

if (WIN32)
    target_link_libraries(BrickSimLib PUBLIC -lImm32)
    target_link_libraries(BrickSimLib PUBLIC -static-libgcc)
    target_link_libraries(BrickSimLib PUBLIC -static-libstdc++)
    target_link_libraries(BrickSimLib PUBLIC -static -pthread)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(BrickSimLib PUBLIC -ltbb)
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON) # useful for fixing build errors

target_compile_definitions(BrickSimLib PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(BrickSimLib PRIVATE USE_SSLEAY)
target_compile_definitions(BrickSimLib PRIVATE USE_OPENSSL)

option(USE_RENDERDOC "Build with RenderDoc in-application API (currently only working on linux)" OFF)
if (USE_RENDERDOC)
    target_compile_definitions(BrickSimLib PRIVATE BRICKSIM_USE_RENDERDOC)
endif ()

option(BRICKSIM_USE_OPTIMIZED_VARIANTS "Use optimized variants of certain functions (to use SSE2 or similar things which make it faster)" ON)
if(BRICKSIM_USE_OPTIMIZED_VARIANTS)
    target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_USE_OPTIMIZED_VARIANTS)
endif()

target_include_directories(BrickSimLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        src/lib/include
        $<INSTALL_INTERFACE:include>)
target_link_libraries(BrickSimLib PUBLIC ${CMAKE_DL_LIBS})

#bash executable
if (EXISTS "C:\\msys64\\usr\\bin\\bash.exe")
    set(BASH_EXECUTABLE "C:\\msys64\\usr\\bin\\bash.exe")
else()
    set(BASH_EXECUTABLE "bash")
endif ()
message(STATUS "bash executable: ${BASH_EXECUTABLE}")

# git statistics
if (NOT DEFINED GIT_COMMIT_HASH)
    execute_process(
            COMMAND ${BASH_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_git_commit_hash.sh"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            RESULT_VARIABLE EXIT_CODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(EXIT_CODE AND NOT EXIT_CODE EQUAL 0)
        message(STATUS "execute_process failed: ${EXIT_CODE}")
    endif()
endif ()

target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_GIT_COMMIT_HASH="${GIT_COMMIT_HASH}")
message(STATUS "BRICKSIM_GIT_COMMIT_HASH=${GIT_COMMIT_HASH}")

if(NOT DEFINED GIT_COMMIT_COUNT)
    execute_process(
            COMMAND ${BASH_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_git_commit_count.sh"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_COUNT
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT})
message(STATUS "BRICKSIM_GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT}")

if(NOT DEFINED TOTAL_HOURS)
    execute_process(
            COMMAND ${BASH_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_git_total_hours.sh"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE TOTAL_HOURS
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_TOTAL_HOURS=${TOTAL_HOURS})
message(STATUS "BRICKSIM_TOTAL_HOURS=${TOTAL_HOURS}")

#version
if (NOT DEFINED GIT_VERSION_TAG)
    execute_process(
            COMMAND ${BASH_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_git_tag.sh"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_VERSION_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif ()

message(STATUS "GIT_VERSION_TAG=${GIT_VERSION_TAG}")
string(REPLACE "v" "" VERSION_TAG_WITHOUT_V "${GIT_VERSION_TAG}")
string(REPLACE "." ";" VERSION_LIST "${VERSION_TAG_WITHOUT_V}")
list(GET VERSION_LIST 0 BRICKSIM_VERSION_MAJOR)
list(GET VERSION_LIST 1 BRICKSIM_VERSION_MINOR)
list(GET VERSION_LIST 2 BRICKSIM_VERSION_PATCH)

target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_VERSION_MAJOR=${BRICKSIM_VERSION_MAJOR})
target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_VERSION_MINOR=${BRICKSIM_VERSION_MINOR})
target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_VERSION_PATCH=${BRICKSIM_VERSION_PATCH})

message(STATUS "BrickSim Version: ${BRICKSIM_VERSION_MAJOR}.${BRICKSIM_VERSION_MINOR}.${BRICKSIM_VERSION_PATCH}")
