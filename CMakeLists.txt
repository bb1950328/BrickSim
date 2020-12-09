# if you are stuck here, take a look at the "How to setup workspace for development" section in README.md
cmake_minimum_required(VERSION 3.16)
project(BrickSim)

file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)#prevent profiles from loading in parallel, causes problems with submodules

set(CMAKE_CXX_STANDARD 17)

if (UNIX)# todo check if this is necessary
    if (NOT APPLE)
        find_package(Threads REQUIRED)
        find_package(X11 REQUIRED)
    endif()
endif()

set(imgui_sources # if cmake doesn't find these files, run `git submodule update --init`
        src/lib/imgui/imconfig.h
        src/lib/imgui/imgui_demo.cpp
        src/lib/imgui/imgui_draw.cpp
        src/lib/imgui/backends/imgui_impl_glfw.cpp
        src/lib/imgui/backends/imgui_impl_glfw.h
        src/lib/imgui/backends/imgui_impl_opengl3.cpp
        src/lib/imgui/backends/imgui_impl_opengl3.h
        src/lib/imgui/imgui_internal.h
        src/lib/imgui/imgui_widgets.cpp
        src/lib/imgui/imgui.cpp
        src/lib/imgui/imgui.h
        src/lib/imgui/imstb_rectpack.h
        src/lib/imgui/imstb_textedit.h
        src/lib/imgui/imstb_truetype.h
        )
set(bricksim_libs
        src/lib/glad.c
        src/lib/stb_image.h
        src/lib/tinyfiledialogs.h
        src/lib/tinyfiledialogs.c
        src/lib/stb_image_write.h
        src/lib/Miniball.hpp
        )

set(bricksim_sources
        src/main.cpp
        src/ldr_files.cpp
        src/ldr_files.h
        src/config.cpp
        src/config.h
        src/test/test_ldr_file_read.cpp
        src/helpers/util.cpp
        src/helpers/util.h
        src/mesh.cpp
        src/mesh.h
        src/helpers/camera.h
        src/helpers/camera.cpp
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
        src/gui/gui.cpp
        src/gui/gui.h
        src/controller.cpp
        src/controller.h
        src/tools/gears.cpp
        src/tools/gears.h
        src/constants.h
        src/helpers/fraction.cpp
        src/helpers/fraction.h
        #src/file_downloader.cpp
        #src/file_downloader.h
        src/git_stats.h
        src/thumbnail_generator.cpp
        src/thumbnail_generator.h
        src/git_stats.cpp
        src/info_providers/part_color_availability_provider.cpp
        src/info_providers/part_color_availability_provider.h
        src/helpers/zip_buffer.cpp
        src/helpers/zip_buffer.h
        src/info_providers/price_guide_provider.cpp
        src/info_providers/price_guide_provider.h
        src/db.cpp
        src/db.h
        src/gui/window_3d.cpp
        src/gui/window_element_tree.cpp
        src/gui/window_element_properties.cpp
        src/gui/window_part_palette.cpp
        src/gui/window_settings.cpp
        src/gui/window_about.cpp
        src/gui/window_sysinfo.cpp
        src/gui/window_debug.cpp
        src/gui/window_orientation_cube.cpp
        src/gui/gui_internal.cpp
        src/gui/gui_internal.h
        src/part_finder.cpp
        src/part_finder.h
        src/orientation_cube.cpp
        src/orientation_cube.h
        src/tasks.cpp src/tasks.h src/types.h)

add_executable(BrickSim
        ${imgui_sources}
        ${bricksim_libs}
        ${bricksim_sources}
        )

if (WIN32)
    option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
    add_subdirectory(src/lib/mingw-std-threads)

    find_package(libzip REQUIRED)
endif ()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# glfw source folder is a git submodule
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/lib/glfw ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/glfw/bin)

# unzip glew-2.1.0.zip to /glew
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/lib/glew/build/cmake)

#todo check if this is also needed on linux
if (WIN32)
    set(SQLITECPP_USE_STACK_PROTECTION OFF)
endif ()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/lib/SQLiteCpp)
include_directories(${CMAKE_CURRENT_LIST_DIR}/src/lib/SQLiteCpp/include)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
message("${LIBZIP_INCLUDE_DIRS}")
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${CURL_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} zip src/lib/imgui)

target_link_libraries(BrickSim ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} glew glfw SQLiteCpp sqlite3)
target_compile_definitions(glew PUBLIC GLEW_STATIC)

target_link_libraries(BrickSim ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS} ${CURL_LIBRARIES} zip)
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

add_compile_options("-lphread")

target_include_directories(BrickSim PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>)