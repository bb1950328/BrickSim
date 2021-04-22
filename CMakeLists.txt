cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

foreach(SDKPATH IN LISTS /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
    if (EXISTS "${SDKPATH}")
        set(CMAKE_OSX_SYSROOT ${SDKPATH})
    endif()
endforeach()
project(BrickSimProject)

file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)#prevent profiles from loading in parallel, causes problems with submodules

#add_library(BrickSimLib OBJECT EXCLUDE_FROM_ALL)
add_library(BrickSimLib STATIC)

#add_executable(BrickSim $<TARGET_OBJECTS:BrickSimLib> src/main.cpp)
add_executable(BrickSim src/main.cpp)
target_link_libraries(BrickSim PRIVATE BrickSimLib)
add_dependencies(BrickSim BrickSimLib)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(BrickSimLib ${OPENGL_LIBRARIES})
if (UNIX)
    if (APPLE)
        target_link_libraries(BrickSimLib "-framework Cocoa")
        target_link_libraries(BrickSimLib "-framework IOKit")
        target_link_libraries(BrickSimLib "-framework CoreVideo")
    endif ()
endif ()

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(src/lib/glfw src/lib/glfw/bin)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(BrickSimLib glfw)
target_link_libraries(BrickSimLib ${GLFW_LIBRARIES})
if (UNIX)# todo check if this is necessary
    if (NOT APPLE)
        find_package(X11 REQUIRED)
        target_link_libraries(BrickSimLib ${X11_LIBRARIES})
    endif ()
endif ()
target_compile_definitions(BrickSimLib PUBLIC GLFW_INCLUDE_NONE)

# GLUT
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
target_link_libraries(BrickSimLib ${GLUT_LIBRARY})

# threads
if (WIN32)
    option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
    add_subdirectory(src/lib/mingw-std-threads)
endif ()
if (UNIX)
    if (NOT APPLE)
        find_package(Threads REQUIRED)
        target_link_libraries(BrickSimLib ${CMAKE_THREAD_LIBS_INIT})
    endif ()
endif ()

# libzip
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)# https://github.com/nih-at/libzip/issues/66
add_subdirectory(src/lib/libzip)
target_include_directories(BrickSimLib PRIVATE zip)
target_link_libraries(BrickSimLib zip)
target_compile_options(zip PRIVATE -Wno-nullability-completeness)

# SqliteCpp
add_subdirectory(src/lib/SQLiteCpp)
include_directories(src/lib/SQLiteCpp/include)
target_link_libraries(BrickSimLib SQLiteCpp)
target_link_libraries(BrickSimLib sqlite3)

# cURL
set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(BrickSimLib ${CURL_LIBRARIES})

# spdlog
add_subdirectory(src/lib/spdlog)
target_include_directories(BrickSimLib PRIVATE spdlog::spdlog)
target_link_libraries(BrickSimLib spdlog::spdlog)

#Catch2
add_subdirectory(src/lib/Catch2)

#GLM
find_package(glm REQUIRED)
target_include_directories(BrickSimLib PRIVATE ${GLM_INCLUDE_DIRS})
#target_link_libraries(BrickSimLib glm)

add_subdirectory(src)

if (WIN32)
    target_link_libraries(BrickSimLib -lssp) # stack smashing protection
    target_link_libraries(BrickSimLib -lImm32)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(BrickSimLib -ltbb)
endif ()

#set(CMAKE_VERBOSE_MAKEFILE ON) # useful for fixing build errors

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
add_compile_definitions(USE_SSLEAY)
add_compile_definitions(USE_OPENSSL)

option(USE_RENDERDOC "Build with RenderDoc in-application API (currently only working on linux)" OFF)
if (USE_RENDERDOC)
    add_compile_definitions(BRICKSIM_USE_RENDERDOC)
endif ()

option(BRICKSIM_USE_OPTIMIZED_VARIANTS "Use optimized variants of certain functions (to use SSE2 or similar things which make it faster)" ON)
if(BRICKSIM_USE_OPTIMIZED_VARIANTS)
    target_compile_definitions(BrickSimLib PUBLIC BRICKSIM_USE_OPTIMIZED_VARIANTS)
endif()

target_include_directories(BrickSimLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        src/lib/include
        $<INSTALL_INTERFACE:include>)
target_link_libraries(BrickSimLib ${CMAKE_DL_LIBS})

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

#packaging
if (UNIX)
    install(TARGETS BrickSim
            RUNTIME)
elseif (WIN32)
    #target_link_libraries(BrickSim -static-libgcc -static-libstdc++)

    install(TARGETS BrickSim
            DESTINATION bin
            COMPONENT BrickSimMain
            )

    find_library(DLL_LIBCURL_LOCATION libcurl)
    find_library(DLL_LIBZIP_LOCATION libzip)
    find_library(DLL_LIBWINPTHREAD_LOCATION winpthread)
    find_library(DLL_LIBGCC_S_SEH_LOCATION libgcc_s_seh)

    message(STATUS ${DLL_LIBCURL_LOCATION})
    message(STATUS ${DLL_LIBZIP_LOCATION})
    message(STATUS ${DLL_LIBWINPTHREAD_LOCATION})
    message(STATUS ${DLL_LIBGCC_S_SEH_LOCATION})

    install(FILES ${DLL_LIBCURL_LOCATION}
            DESTINATION bin
            COMPONENT BrickSimMain
            )
    install(FILES ${DLL_LIBZIP_LOCATION}
            DESTINATION bin
            COMPONENT BrickSimMain
            )

    set(CPACK_COMPONENTS_ALL BrickSimMain)
    set(CPACK_COMPONENT_BrickSimMain_DISPLAY_NAME "BrickSim Application")

    # no clue where these components are coming from...
    set(CPACK_COMPONENT_Unspecified_HIDDEN TRUE)
    set(CPACK_COMPONENT_headers_HIDDEN TRUE)
    set(CPACK_COMPONENT_libraries_HIDDEN TRUE)

endif ()

set(CPACK_PACKAGE_NAME "BrickSim")
set(CPACK_PACKAGE_EXECUTABLES "BrickSim" "BrickSim")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BrickSim")
set(CPACK_PACKAGE_VENDOR "bb1950328")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${BRICKSIM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${BRICKSIM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${BRICKSIM_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "BrickSim_${BRICKSIM_VERSION_MAJOR}_${BRICKSIM_VERSION_MINOR}_${BRICKSIM_VERSION_PATCH}")

set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/resources\\\\logo_icon.png")

#set(CPACK_INSTALL_CMAKE_PROJECTS BrickSim)

if (WIN32)
    SET(CPACK_GENERATOR "WIX")

    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/resources/logo_icon.ico")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER BrickSim)

    # There is a bug in NSI that does not handle full UNIX paths properly.
    # Make sure there is at least one set of four backlashes.
    #SET(CPACK_NSIS_MUI_ICON "${CMake_SOURCE_DIR}/resources\\\\logo_icon.ico")
    #SET(CPACK_NSIS_MUI_UNIICON "${CMake_SOURCE_DIR}/resources\\\\logo_icon_uninstall.ico")

    #set(CPACK_NSIS_MENU_LINKS "programs\\bin\\BrickSim" "BrickSim")

    ###set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
    #set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} BrickSim")
    #set(CPACK_NSIS_HELP_LINK "https:\\\\\\\\bb1950328.github.io/BrickSim/")
    #set(CPACK_NSIS_URL_INFO_ABOUT "https:\\\\\\\\bb1950328.github.io/BrickSim/")
    ##set(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
endif ()

if (UNIX)
    set(CPACK_PACKAGE_CONTACT bb1950328@gmail.com)
    set(CPACK_PACKAGE_FILE_NAME BrickSim)
    if(APPLE)
        set(CPACK_BUNDLE_NAME BrickSim)
        set(CPACK_BUNDLE_ICON "${CMake_SOURCE_DIR}/resources/logo_icon.png")
        set(CPACK_BUNDLE_PLIST "${CMake_SOURCE_DIR}/resources/Info.plist")
        set(CPACK_BUNDLE_STARTUP_COMMAND BrickSim) # TODO maybe a startup script is needed, https://gitlab.kitware.com/cmake/community/-/wikis/doc/cpack/PackageGenerators#bundle-osx-only has more info
    else()#Linux
        SET(CPACK_GENERATOR "DEB")

        #For some reason, shlibdeps doesn't work on the GitHub Actions server. that's why the dependencies are hardcoded
        #uncomment the next line, run cpack locally and run 'dpkg --info _CPack_Packages/Linux/DEB/BrickSim.deb' if you want to update the dependencies
        #set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.29), libcurl4 (>= 7.16.2), libgcc-s1 (>= 3.0), libgl1, libstdc++6 (>= 9), libx11-6, libzip5 (>= 0.10), libopengl0")
    endif()
endif ()

include(CPack)
