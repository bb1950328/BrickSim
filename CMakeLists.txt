cmake_minimum_required(VERSION 3.16)
project(BrickSim)

set(CMAKE_CXX_STANDARD 14)

add_executable(BrickSim main.cpp glad.c)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

target_link_libraries(BrickSim ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} glfw)
#target_link_libraries(-lglfw)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(BrickSim ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

target_include_directories(BrickSim PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>)