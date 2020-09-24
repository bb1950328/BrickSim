cmake_minimum_required(VERSION 3.16)
project(BrickSim)

set(CMAKE_CXX_STANDARD 14)

add_executable(BrickSim src/main.cpp src/lib/glad.c src/ldr_objects.cpp src/ldr_objects.h src/config.cpp src/config.h src/test/test_configuration_reader.cpp src/test/test_ldr_file_read.cpp src/util.cpp src/util.h src/lib/stb_image.h src/mesh.cpp src/mesh.h src/camera.h src/camera.cpp)

# On Ubuntu, I had to sudo apt install mesa-utils freeglut3-dev libglfw3-dev
# and add the files from https://glad.dav1d.de to /usr/include and the project folder
# windows libraries and headers installation: https://learnopengl.com/Getting-started/Creating-a-window

IF(MINGW)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    # glfw source folder is sibling of BrickSim folder
    # https://www.glfw.org/download.html -> click on "Source package"
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../glfw-3.3.2 ${CMAKE_CURRENT_SOURCE_DIR}/../glfw-3.3.2/bin)
ELSE()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
ENDIF()

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

target_link_libraries(BrickSim ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} glfw)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(BrickSim ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

target_include_directories(BrickSim PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>)