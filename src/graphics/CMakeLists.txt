target_sources(BrickSimLib PRIVATE
        camera.cpp
        camera.h
        orientation_cube.cpp
        orientation_cube.h
        overlay_2d.cpp
        overlay_2d.h
        scene.cpp
        scene.h
        shaders.cpp
        shaders.h
        texture.cpp
        texture.h
        thumbnail_generator.cpp
        thumbnail_generator.h
        )

add_subdirectory(mesh)