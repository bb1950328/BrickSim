cmake_minimum_required(VERSION 3.16)
project(BrickSim)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE "Release")

if (UNIX)# todo check if this is necessary
    if (NOT APPLE)
        find_package(Threads REQUIRED)
        find_package(X11 REQUIRED)
    endif()
endif()

set(imgui_sources
        imgui/imconfig.h
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/examples/imgui_impl_glfw.cpp
        imgui/examples/imgui_impl_glfw.h
        imgui/examples/imgui_impl_opengl3.cpp
        imgui/examples/imgui_impl_opengl3.h
        imgui/imgui_internal.h
        imgui/imgui_widgets.cpp
        imgui/imgui.cpp
        imgui/imgui.h
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        )

set(bricksim_sources
        src/main.cpp
        src/lib/glad.c
        src/ldr_files.cpp
        src/ldr_files.h
        src/config.cpp
        src/config.h
        src/test/test_configuration_reader.cpp
        src/test/test_ldr_file_read.cpp
        src/util.cpp
        src/util.h
        src/lib/stb_image.h
        src/mesh.cpp
        src/mesh.h
        src/camera.h
        src/camera.cpp
        src/shaders/shader.cpp
        src/element_tree.cpp
        src/element_tree.h
        src/mesh_collection.cpp
        src/mesh_collection.h
        src/statistic.cpp
        src/statistic.h
        src/ldr_file_repository.cpp
        src/ldr_file_repository.h
        src/ldr_colors.cpp
        src/ldr_colors.h
        src/renderer.cpp
        src/renderer.h
        src/gui.cpp
        src/gui.h
        )

add_executable(BrickSim
        ${imgui_sources}
        ${bricksim_sources}
        )

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
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} imgui)

target_link_libraries(BrickSim ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} glfw)
include_directories(${GLFW_INCLUDE_DIRS})

target_link_libraries(BrickSim ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})
if (UNIX)#todo check if this is needed
    if (APPLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
                )
    else()
        target_link_libraries(${CMAKE_PROJECT_NAME}
                ${CMAKE_THREAD_LIBS_INIT}
                ${X11_LIBRARIES}
                ${CMAKE_DL_LIBS}
                )
    endif()
endif()


target_include_directories(BrickSim PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>)