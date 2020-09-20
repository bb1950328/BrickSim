cmake_minimum_required(VERSION 3.16)
project(BrickSim)

set(CMAKE_CXX_STANDARD 14)

add_executable(BrickSim main.cpp glad.c src/ldr_objects.cpp src/ldr_objects.h src/config.cpp src/config.h src/test/test_configuration_reader.cpp src/test/test_ldr_file_read.cpp src/util.cpp src/util.h)

# On Ubuntu, I had to sudo apt install mesa-utils freeglut3-dev libglfw3-dev
# and add the files from https://glad.dav1d.de to /usr/include and the project folder

# maybe this is helpful: https://stackoverflow.com/questions/9160335/os-specific-instructions-in-cmake-how-to

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

target_link_libraries(BrickSim ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} glfw)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(BrickSim ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

target_include_directories(BrickSim PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>)