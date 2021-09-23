target_sources(BrickSimLib PRIVATE
        window_view3d.cpp
        window_view3d.h
        window_about.cpp
        window_about.h
        window_debug.cpp
        window_debug.h
        window_editor_meta_info.cpp
        window_editor_meta_info.h
        window_element_properties.cpp
        window_element_properties.h
        window_element_tree.cpp
        window_element_tree.h
        window_log.cpp
        window_log.h
        window_mesh_inspector.cpp
        window_mesh_inspector.h
        window_model_info.cpp
        window_model_info.h
        window_orientation_cube.cpp
        window_orientation_cube.h
        window_part_palette.cpp
        window_part_palette.h
        window_settings.cpp
        window_settings.h
        window_system_info.cpp
        window_system_info.h

        windows.cpp
        windows.h
        )

add_subdirectory(tools)