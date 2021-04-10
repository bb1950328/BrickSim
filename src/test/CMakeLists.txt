add_executable(BrickSimTests
        catchMain.cpp
        testUtil.cpp
        )
target_link_libraries(BrickSimTests PRIVATE Catch2::Catch2)
add_dependencies(BrickSimTests BrickSimLib)
target_link_libraries(BrickSimTests PRIVATE BrickSimLib)