target_sources(BrickSimLib PRIVATE
        gui.cpp
        gui.h
        gui_internal.cpp
        gui_internal.h
        icons.cpp
        icons.h
        modals.cpp
        modals.h
        )
add_subdirectory(context_menu)
add_subdirectory(graphical_transform)
add_subdirectory(windows)
