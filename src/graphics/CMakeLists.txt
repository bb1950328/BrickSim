target_sources(BrickSimLib PRIVATE
        camera.cpp
        camera.h
        connection_visualization.cpp
        connection_visualization.h
        hardware_properties.cpp
        hardware_properties.h
        opengl_native_or_replacement.cpp
        opengl_native_or_replacement.h
        orientation_cube.cpp
        orientation_cube.h
        scene.cpp
        scene.h
        shaders.cpp
        shaders.h
        texmap_projection.cpp
        texmap_projection.h
        texture.cpp
        texture.h
        )

add_subdirectory(mesh)
add_subdirectory(overlay2d)
add_subdirectory(thumbnail)
