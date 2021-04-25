set(imgui_sources
        imgui/imconfig.h
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_glfw.h
        imgui/backends/imgui_impl_opengl3.cpp
        imgui/backends/imgui_impl_opengl3.h
        imgui/imgui_internal.h
        imgui/imgui_widgets.cpp
        imgui/imgui.cpp
        imgui/imgui.h
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        imgui/imgui_tables.cpp
        )
set(other_libs
        glad.c
        Miniball.hpp
        IconFontCppHeaders/IconsFontAwesome5.h
        )

target_sources(BrickSimLib PUBLIC
        ${imgui_sources}
        ${other_libs}
        )

target_include_directories(BrickSimLib PUBLIC imgui)
add_compile_definitions(IMGUI_DISABLE_OBSOLETE_FUNCTIONS)