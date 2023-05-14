target_sources(BrickSimLib PRIVATE
        connector_data_provider.cpp
        connector_data_provider.h
        bounding.cpp
        bounding.h
        data.cpp
        data.h
        engine.cpp
        engine.h
        )

add_subdirectory(ldcad_meta)
add_subdirectory(visualization)
