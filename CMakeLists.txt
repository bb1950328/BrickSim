cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

foreach (SDKPATH IN LISTS /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
    if (EXISTS "${SDKPATH}")
        set(CMAKE_OSX_SYSROOT ${SDKPATH})
    endif ()
endforeach ()

#bash executable
if (EXISTS "C:\\msys64\\usr\\bin\\bash.exe")
    set(BASH_EXECUTABLE "C:\\msys64\\usr\\bin\\bash.exe")
else ()
    set(BASH_EXECUTABLE "bash")
endif ()
message(STATUS "bash executable: ${BASH_EXECUTABLE}")

execute_process(
        COMMAND ${BASH_EXECUTABLE} "--version"
        OUTPUT_VARIABLE BASH_VERSION_OUTPUT
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE EXIT_CODE
)
if (EXIT_CODE AND NOT EXIT_CODE EQUAL 0)
    message(WARNING "calling bash failed with exit code ${EXIT_CODE}")
    set(GIT_COMMIT_HASH "unknown")
    set(GIT_COMMIT_COUNT "0")
    set(TOTAL_HOURS "0.0")
    set(GIT_VERSION_TAG "v0.0.0")
else()
    if (NOT DEFINED GIT_COMMIT_HASH)
        execute_process(
                COMMAND ${BASH_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_git_commit_hash.sh"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_COMMIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif ()


    if (NOT DEFINED GIT_COMMIT_COUNT)
        execute_process(
                COMMAND ${BASH_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_git_commit_count.sh"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_COMMIT_COUNT
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif ()

    if (NOT DEFINED TOTAL_HOURS)
        execute_process(
                COMMAND ${BASH_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_git_total_hours.sh"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE TOTAL_HOURS
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif ()

    if (NOT DEFINED GIT_VERSION_TAG)
        execute_process(
                COMMAND ${BASH_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/get_git_tag.sh"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_VERSION_TAG
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif ()
endif ()

message(STATUS "BrickSim git commit hash:       ${GIT_COMMIT_HASH}")
message(STATUS "BrickSim git commit count:      ${GIT_COMMIT_COUNT}")
message(STATUS "BrickSim total hours:           ${TOTAL_HOURS}")
message(STATUS "BrickSim git version tag:       ${GIT_VERSION_TAG}")
string(REPLACE "v" "" GIT_VERSION_TAG_WITHOUT_V "${GIT_VERSION_TAG}")

project(BrickSim
        VERSION "${GIT_VERSION_TAG_WITHOUT_V}"
        HOMEPAGE_URL "https://www.bricksim.org")

message(STATUS "BrickSim parsed version:        ${BrickSim_VERSION_MAJOR}.${BrickSim_VERSION_MINOR}.${BrickSim_VERSION_PATCH}")

#file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)#prevent profiles from loading in parallel, causes problems with submodules

#set(CMAKE_VERBOSE_MAKEFILE ON) # useful for fixing build errors

add_library(BrickSimLib STATIC)

option(BRICKSIM_USE_ASAN "Use address sanitizer" OFF) # enabling this makes the program around 40% slower
if (BRICKSIM_USE_ASAN)
    #these libraries have to be linked before all other libraries
    target_link_libraries(BrickSimLib PUBLIC -lasan)
    target_compile_options(BrickSimLib PUBLIC -fsanitize=address)
endif ()

add_executable(BrickSim src/main.cpp)
target_link_libraries(BrickSim PRIVATE BrickSimLib)

add_executable(BrickSimTests src/test/catchMain.cpp)
target_link_libraries(BrickSimTests PRIVATE BrickSimLib)

target_compile_definitions(BrickSimLib PUBLIC
        BRICKSIM_TOTAL_HOURS=${TOTAL_HOURS}
        BRICKSIM_GIT_COMMIT_COUNT=${GIT_COMMIT_COUNT}
        BRICKSIM_GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
        BRICKSIM_VERSION_MAJOR=${BrickSim_VERSION_MAJOR}
        BRICKSIM_VERSION_MINOR=${BrickSim_VERSION_MINOR}
        BRICKSIM_VERSION_PATCH=${BrickSim_VERSION_PATCH}
        )

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
target_compile_definitions(BrickSimLib PUBLIC GLFW_INCLUDE_NONE)

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
if (APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
    find_package(openssl REQUIRED)
    message(STATUS "openssl found in main CMakeLists.txt")
endif ()
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
target_compile_definitions(BrickSimTests PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
target_link_libraries(BrickSimTests PRIVATE Catch2::Catch2)

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

#cpuinfo
set(CPUINFO_BUILD_TOOLS OFF CACHE BOOL "Build command-line tools" FORCE)
set(CPUINFO_BUILD_UNIT_TESTS OFF CACHE BOOL "Build cpuinfo unit tests" FORCE)
set(CPUINFO_BUILD_MOCK_TESTS OFF CACHE BOOL "Build cpuinfo mock tests" FORCE)
set(CPUINFO_BUILD_BENCHMARKS OFF CACHE BOOL "Build cpuinfo micro-benchmarks" FORCE)
add_subdirectory(src/lib/cpuinfo)
target_link_libraries(BrickSimLib PRIVATE cpuinfo)

#magic_enum
add_subdirectory(src/lib/magic_enum)
target_link_libraries(BrickSimLib PRIVATE magic_enum::magic_enum)

add_subdirectory(src)

if (WIN32)
    target_link_libraries(BrickSimLib PUBLIC -lImm32)
    target_link_libraries(BrickSimLib PUBLIC -static-libgcc)
    target_link_libraries(BrickSimLib PUBLIC -static-libstdc++)
    target_link_libraries(BrickSimLib PUBLIC -static -pthread)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(BrickSimLib PUBLIC -ltbb)
endif ()

target_compile_definitions(BrickSimLib PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(BrickSimLib PRIVATE USE_SSLEAY)
target_compile_definitions(BrickSimLib PRIVATE USE_OPENSSL)

option(USE_RENDERDOC "Build with RenderDoc in-application API (currently only working on linux)" OFF)
if (USE_RENDERDOC)
    target_compile_definitions(BrickSimLib PRIVATE BRICKSIM_USE_RENDERDOC)
endif ()

option(BRICKSIM_USE_OPTIMIZED_VARIANTS "Use optimized variants of certain functions (to use SSE2 or similar things which make it faster)" ON)
if (BRICKSIM_USE_OPTIMIZED_VARIANTS)
    target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_USE_OPTIMIZED_VARIANTS)
endif ()

target_include_directories(BrickSimLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        src/lib/include
        $<INSTALL_INTERFACE:include>)
target_link_libraries(BrickSimLib PUBLIC ${CMAKE_DL_LIBS})

