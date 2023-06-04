target_sources(BrickSimTests PRIVATE
        test_user_actions.cpp
        testing_tools.h
        )

add_subdirectory(connection)
add_subdirectory(graphics)
add_subdirectory(gui)
add_subdirectory(helpers)
add_subdirectory(ldr)
add_subdirectory(tools)
