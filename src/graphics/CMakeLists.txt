target_sources(BrickSimLib PRIVATE
        camera.cpp
        camera.h
        opengl_native_or_replacement.cpp
        opengl_native_or_replacement.h
        orientation_cube.cpp
        orientation_cube.h
        overlay_2d.cpp
        overlay_2d.h
        scene.cpp
        scene.h
        shaders.cpp
        shaders.h
        texmap_projection.cpp
        texmap_projection.h
        texture.cpp
        texture.h
        thumbnail_generator.cpp
        thumbnail_generator.h
        )

add_subdirectory(mesh)