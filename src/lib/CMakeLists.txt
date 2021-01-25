set(imgui_sources # if cmake doesn't find these files, run `git submodule update --init`
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
        stb_image.h
        tinyfiledialogs.h
        tinyfiledialogs.c
        stb_image_write.h
        Miniball.hpp
        IconFontCppHeaders/IconsFontAwesome5.h
        )

set(rapidjson_sources
        rapidjson/allocators.h
        rapidjson/cursorstreamwrapper.h
        rapidjson/document.h
        rapidjson/encodedstream.h
        rapidjson/encodings.h
        rapidjson/error
        rapidjson/filereadstream.h
        rapidjson/filewritestream.h
        rapidjson/fwd.h
        rapidjson/internal
        rapidjson/istreamwrapper.h
        rapidjson/memorybuffer.h
        rapidjson/memorystream.h
        rapidjson/msinttypes
        rapidjson/ostreamwrapper.h
        rapidjson/pointer.h
        rapidjson/prettywriter.h
        rapidjson/rapidjson.h
        rapidjson/reader.h
        rapidjson/schema.h
        rapidjson/stream.h
        rapidjson/stringbuffer.h
        rapidjson/writer.h

        rapidjson/error/en.h
        rapidjson/error/error.h

        rapidjson/internal/biginteger.h
        rapidjson/internal/clzll.h
        rapidjson/internal/diyfp.h
        rapidjson/internal/dtoa.h
        rapidjson/internal/ieee754.h
        rapidjson/internal/itoa.h
        rapidjson/internal/meta.h
        rapidjson/internal/pow10.h
        rapidjson/internal/regex.h
        rapidjson/internal/stack.h
        rapidjson/internal/strfunc.h
        rapidjson/internal/strtod.h
        rapidjson/internal/swap.h

        rapidjson/msinttypes/inttypes.h
        rapidjson/msinttypes/stdint.h
        )

target_sources(BrickSim PUBLIC
        ${imgui_sources}
        ${other_libs}
        ${rapidjson_sources}
        )

target_include_directories(BrickSim PUBLIC imgui)
add_compile_definitions(IMGUI_DISABLE_OBSOLETE_FUNCTIONS)