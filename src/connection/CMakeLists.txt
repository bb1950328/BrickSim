target_sources(BrickSimLib PRIVATE
        connector_data_provider.cpp
        connector_data_provider.h
        connector_data_visualizer.cpp
        connector_data_visualizer.h
        bounding.cpp
        bounding.h
        data.cpp
        data.h
        engine.cpp
        engine.h
        )

add_subdirectory(ldcad_snap_meta)
