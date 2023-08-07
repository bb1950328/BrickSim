target_sources(BrickSimLib PRIVATE
        connector_data_provider.cpp
        connector_data_provider.h
        bounding.cpp
        bounding.h
        connection_check.cpp
        connection_check.h
        connection_graph.cpp
        connection_graph.h
        connector_conversion.cpp
        connector_conversion.h
        connection.cpp
        connection.h
        engine.cpp
        engine.h
        pair_checker.cpp
        pair_checker.h
        )

add_subdirectory(connector)
add_subdirectory(ldcad_meta)
add_subdirectory(visualization)
