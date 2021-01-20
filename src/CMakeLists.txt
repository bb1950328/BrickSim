target_sources(BrickSim PUBLIC
        main.cpp
        ldr_files.cpp
        ldr_files.h
        config.cpp
        config.h
        mesh.cpp
        mesh.h
        shaders/shader.cpp
        element_tree.cpp
        element_tree.h
        mesh_collection.cpp
        mesh_collection.h
        statistic.cpp
        statistic.h
        ldr_file_repository.cpp
        ldr_file_repository.h
        ldr_colors.cpp
        ldr_colors.h
        renderer.cpp
        renderer.h

        controller.cpp
        controller.h
        tools/gears.cpp
        tools/gears.h
        constant_data/constants.h

        #file_downloader.cpp
        #file_downloader.h
        constant_data/git_stats.h
        thumbnail_generator.cpp
        thumbnail_generator.h
        constant_data/git_stats.cpp



        db.cpp
        db.h

        part_finder.cpp
        part_finder.h
        orientation_cube.cpp
        orientation_cube.h
        tasks.cpp
        tasks.h
        types.h
        constant_data/resources.cpp
        constant_data/resources.h

        overlay_2d.cpp
        overlay_2d.h
        latest_log_messages_tank.cpp
        latest_log_messages_tank.h
        )

add_subdirectory(info_providers)
add_subdirectory(helpers)
add_subdirectory(gui)
add_subdirectory(lib)