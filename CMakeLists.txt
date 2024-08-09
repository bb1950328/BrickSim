cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

foreach (SDKPATH IN LISTS /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
    if (EXISTS "${SDKPATH}")
        set(CMAKE_OSX_SYSROOT ${SDKPATH})
    endif ()
endforeach ()

if (MSVC)
    message(WARNING "getting git version info currently not implemented on MSVC")
else ()
    #todo automatically search for bash in $PATH
    if (EXISTS "C:\\msys64\\usr\\bin\\bash.exe")
        set(BASH_EXECUTABLE "C:\\msys64\\usr\\bin\\bash.exe")
    elseif(EXISTS "C:\\Program Files\\Git\\bin\\bash.exe")
        set(BASH_EXECUTABLE "C:\\Program Files\\Git\\bin\\bash.exe")
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
    else ()
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
endif ()

if (NOT DEFINED GIT_COMMIT_HASH)
    set(GIT_COMMIT_HASH "unknown")
endif ()
if (NOT DEFINED GIT_COMMIT_COUNT)
    set(GIT_COMMIT_COUNT "0")
endif ()
if (NOT DEFINED TOTAL_HOURS)
    set(TOTAL_HOURS "0.0")
endif ()
if (NOT DEFINED GIT_VERSION_TAG)
    set(GIT_VERSION_TAG "v0.0.0")
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

add_executable(BrickSimBenchmarks src/benchmarks/benchMain.cpp)
target_link_libraries(BrickSimBenchmarks PRIVATE BrickSimLib)

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
set(ENABLE_BZIP2 OFF CACHE BOOL "Enable use of BZip2" FORCE)
set(ENABLE_LZMA OFF CACHE BOOL "Enable use of LZMA" FORCE)
set(ENABLE_ZSTD OFF CACHE BOOL "Enable use of Zstandard" FORCE)
set(BUILD_REGRESS OFF CACHE BOOL "Build regression tests" FORCE)
set(BUILD_DOC OFF CACHE BOOL "Build documentation" FORCE)
add_subdirectory(src/lib/libzip)
target_link_libraries(BrickSimLib PUBLIC libzip::zip)
if (NOT MSVC)
    target_compile_options(zip PRIVATE -Wno-nullability-completeness)
endif ()

# SqliteCpp
set(SQLITECPP_USE_STACK_PROTECTION OFF)
add_subdirectory(src/lib/SQLiteCpp)
target_include_directories(BrickSimLib PUBLIC src/lib/SQLiteCpp/include)
target_link_libraries(BrickSimLib PRIVATE SQLiteCpp)
target_link_libraries(BrickSimLib PRIVATE sqlite3)

# cURL
#if (APPLE)
#    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
#    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
#    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
#    set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
#    find_package(openssl REQUIRED)
#    message(STATUS "openssl found in main CMakeLists.txt")
#endif ()
set(HTTP_ONLY ON)
set(CURL_USE_LIBSSH2 OFF)
set(CURL_DISABLE_TESTS ON)
if (APPLE)
    set(CURL_USE_SECTRANSP ON CACHE BOOL "enable Apple OS native SSL/TLS" FORCE)
elseif (WIN32)
    set(CURL_USE_SCHANNEL ON CACHE BOOL "enable Windows native SSL/TLS" FORCE)
endif ()
add_subdirectory(src/lib/curl)
target_link_libraries(BrickSimLib PUBLIC CURL::libcurl)

# spdlog
add_subdirectory(src/lib/spdlog)
target_include_directories(BrickSimLib PUBLIC spdlog::spdlog)
target_link_libraries(BrickSimLib PUBLIC spdlog::spdlog)
# target_link_libraries(BrickSimLib PUBLIC fmt)

#Catch2
add_subdirectory(src/lib/Catch2)
target_link_libraries(BrickSimTests PRIVATE Catch2::Catch2)
target_compile_definitions(BrickSimBenchmarks PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
target_link_libraries(BrickSimBenchmarks PRIVATE Catch2::Catch2)

#GLM
add_subdirectory(src/lib/glm)
target_link_libraries(BrickSimLib PUBLIC glm::glm)
target_compile_definitions(BrickSimLib PUBLIC GLM_ENABLE_EXPERIMENTAL)

#tinyfiledialogs
target_include_directories(BrickSimLib PUBLIC src/lib/tinyfiledialogs)
target_sources(BrickSimLib PRIVATE src/lib/tinyfiledialogs/tinyfiledialogs.c)

#rapidjson
target_include_directories(BrickSimLib PUBLIC src/lib/rapidjson/include)

#stb
target_include_directories(BrickSimLib PUBLIC src/lib/stb)
target_compile_definitions(BrickSimLib PUBLIC "STBI_MAX_DIMENSIONS=(1<<31)")

#cpuinfo
set(CPUINFO_BUILD_TOOLS OFF CACHE BOOL "Build command-line tools" FORCE)
set(CPUINFO_BUILD_UNIT_TESTS OFF CACHE BOOL "Build cpuinfo unit tests" FORCE)
set(CPUINFO_BUILD_MOCK_TESTS OFF CACHE BOOL "Build cpuinfo mock tests" FORCE)
set(CPUINFO_BUILD_BENCHMARKS OFF CACHE BOOL "Build cpuinfo micro-benchmarks" FORCE)
add_subdirectory(src/lib/cpuinfo)
target_link_libraries(BrickSimLib PRIVATE cpuinfo)

#magic_enum
add_subdirectory(src/lib/magic_enum)
target_link_libraries(BrickSimLib PUBLIC magic_enum::magic_enum)

#fast_float
add_subdirectory(src/lib/fast_float)
target_link_libraries(BrickSimLib PRIVATE fast_float)

#unordered_dense
add_subdirectory(src/lib/unordered_dense)
target_link_libraries(BrickSimLib PUBLIC unordered_dense::unordered_dense)

#efsw
set(CXX11 ON CACHE BOOL "Build efsw with C++11" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build efsw as a shared library" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "Build efsw as a static library" FORCE)
set(EFSW_INSTALL OFF CACHE BOOL "Add efsw install targets" FORCE)
add_subdirectory(src/lib/efsw)
target_link_libraries(BrickSimLib PUBLIC efsw)
#target_include_directories(BrickSimLib PRIVATE src/lib/efsw/include)

#palanteer
option(USE_PALANTEER "Build with Palanteer instrumentation library" OFF)
if (USE_PALANTEER)
    target_compile_definitions(BrickSimLib PRIVATE USE_PL=1)
endif ()
target_include_directories(BrickSimLib PRIVATE src/lib/palanteer/c++)

#fcl
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
set(FCL_STATIC_LIBRARY ON CACHE BOOL "Whether the FCL library should be static rather than shared" FORCE)
set(FCL_WITH_OCTOMAP OFF CACHE BOOL "OctoMap library support" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Build the testing tree." FORCE)
add_subdirectory(src/lib/fcl)
target_link_libraries(BrickSimLib PUBLIC fcl)
#target_include_directories(BrickSimLib PUBLIC src/lib/fcl/include)

#eigen
find_package(Eigen3 3.4 REQUIRED NO_MODULE)
target_link_libraries(BrickSimLib PUBLIC Eigen3::Eigen)

#ImGuiColorTextEdit
add_library(ImGuiColorTextEdit)
target_sources(ImGuiColorTextEdit PRIVATE
        src/lib/ImGuiColorTextEdit/TextEditor.cpp
        src/lib/ImGuiColorTextEdit/TextEditor.h
)
target_compile_definitions(ImGuiColorTextEdit PRIVATE IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
target_include_directories(ImGuiColorTextEdit PRIVATE src/lib/imgui)
target_include_directories(ImGuiColorTextEdit INTERFACE src/lib/ImGuiColorTextEdit)

target_link_libraries(BrickSimLib PRIVATE ImGuiColorTextEdit)

#UTF8-CPP
add_subdirectory(src/lib/utfcpp)
target_link_libraries(BrickSimLib PUBLIC utf8cpp)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(utf8cpp INTERFACE "/Zc:__cplusplus") # https://stackoverflow.com/a/60890947/8733066
endif()

#miniball
target_include_directories(BrickSimLib PUBLIC src/lib/miniball/cpp/main)

#json_dto
add_subdirectory(src/lib/json_dto/dev)
target_link_libraries(BrickSimLib PUBLIC json-dto)

#Clipper2
option(CLIPPER2_UTILS "Build utilities" OFF)
option(CLIPPER2_EXAMPLES "Build examples" OFF)
option(CLIPPER2_TESTS "Build tests" OFF)
add_subdirectory(src/lib/Clipper2/CPP)
target_link_libraries(BrickSimLib PUBLIC Clipper2)

#pugixml
add_subdirectory(src/lib/pugixml)
target_link_libraries(BrickSimLib PUBLIC pugixml-static)

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

if (MSVC)
    target_compile_definitions(BrickSimLib PUBLIC _USE_MATH_DEFINES)
    target_compile_options(BrickSimLib PUBLIC /wd4068)# warning C4068: unknown pragma
    set_property(TARGET BrickSim PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif ()

target_include_directories(BrickSimLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        src/lib/include
        $<INSTALL_INTERFACE:include>)
target_link_libraries(BrickSimLib PUBLIC ${CMAKE_DL_LIBS})

