target_sources(BrickSimLib PRIVATE
        bounding.cpp
        bounding.h
        connection.cpp
        connection.h
        connection_check.cpp
        connection_check.h
        connection_graph.cpp
        connection_graph.h
        connector_conversion.cpp
        connector_conversion.h
        connector_data_provider.cpp
        connector_data_provider.h
        degrees_of_freedom.cpp
        degrees_of_freedom.h
        engine.cpp
        engine.h
        intersection_graph.cpp
        intersection_graph.h
        pair_checker.cpp
        pair_checker.h
        )

add_subdirectory(connector)
add_subdirectory(ldcad_meta)
add_subdirectory(visualization)
