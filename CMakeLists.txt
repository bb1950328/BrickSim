# if you are stuck here, take a look at the "How to setup workspace for development" section in README.md
cmake_minimum_required(VERSION 3.16)
project(BrickSim)

file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)#prevent profiles from loading in parallel, causes problems with submodules

set(BRICKSIM_VERSION_MAJOR 1)
set(BRICKSIM_VERSION_MINOR 0)
set(BRICKSIM_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)

set(OpenGL_GL_PREFERENCE GLVND)

if (UNIX)# todo check if this is necessary
    if (NOT APPLE)
        find_package(Threads REQUIRED)
        find_package(X11 REQUIRED)
    endif()
endif()

add_executable(BrickSim)

add_subdirectory(src)

if (WIN32)
    option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
    add_subdirectory(src/lib/mingw-std-threads)

    find_package(libzip REQUIRED)
endif ()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(src/lib/glfw src/lib/glfw/bin)

add_subdirectory(src/lib/glew/build/cmake)

add_subdirectory(src/lib/SQLiteCpp)
include_directories(src/lib/SQLiteCpp/include)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
message("LIBZIP_INCLUDE_DIRS = ${LIBZIP_INCLUDE_DIRS}")
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${CURL_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} zip src/lib/imgui)

target_link_libraries(BrickSim ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} glew glfw SQLiteCpp sqlite3)
target_compile_definitions(glew PUBLIC GLEW_STATIC)

target_link_libraries(BrickSim ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS} ${CURL_LIBRARIES})

if(WIN32)
    target_link_libraries(BrickSim -lssp)
endif()

if(UNIX)
    target_link_libraries(BrickSim libzip.so.5)
else()#todo find out which one for apple
    target_link_libraries(BrickSim zip)
endif()

if (UNIX)#todo check if this is needed
    if (APPLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
                "-framework Cocoa"
                "-framework IOKit"
                "-framework CoreVideo"
                )
    else()
        target_link_libraries(${CMAKE_PROJECT_NAME}
                ${CMAKE_THREAD_LIBS_INIT}
                ${X11_LIBRARIES}
                ${CMAKE_DL_LIBS}
                )
    endif()
endif()

add_compile_options("-lphread")

target_include_directories(BrickSim PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        src/lib/include
        $<INSTALL_INTERFACE:include>)


#packaging
if (UNIX)
    install(TARGETS BrickSim
            RUNTIME)
elseif(WIN32)
    #target_link_libraries(BrickSim -static-libgcc -static-libstdc++)

    install(TARGETS BrickSim
            DESTINATION bin
            COMPONENT BrickSimMain
            )

    find_library(DLL_LIBCURL_LOCATION libcurl)
    find_library(DLL_LIBZIP_LOCATION libzip)
    find_library(DLL_LIBWINPTHREAD_LOCATION winpthread)
    find_library(DLL_LIBGCC_S_SEH_LOCATION libgcc_s_seh)

    message(STATUS ${DLL_LIBCURL_LOCATION})
    message(STATUS ${DLL_LIBZIP_LOCATION})
    message(STATUS ${DLL_LIBWINPTHREAD_LOCATION})
    message(STATUS ${DLL_LIBGCC_S_SEH_LOCATION})

    install(FILES ${DLL_LIBCURL_LOCATION}
            DESTINATION bin
            COMPONENT BrickSimMain
            )
    install(FILES ${DLL_LIBZIP_LOCATION}
            DESTINATION bin
            COMPONENT BrickSimMain
            )

    set(CPACK_COMPONENTS_ALL BrickSimMain)
    set(CPACK_COMPONENT_BrickSimMain_DISPLAY_NAME "BrickSim Application")

    # no clue where these components are coming from...
    set(CPACK_COMPONENT_Unspecified_HIDDEN TRUE)
    set(CPACK_COMPONENT_headers_HIDDEN TRUE)
    set(CPACK_COMPONENT_libraries_HIDDEN TRUE)

endif ()

set(CPACK_PACKAGE_NAME "BrickSim")
set(CPACK_PACKAGE_EXECUTABLES "BrickSim" "BrickSim")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BrickSim")
set(CPACK_PACKAGE_VENDOR "bb1950328")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${BRICKSIM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${BRICKSIM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${BRICKSIM_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "BrickSim_${BRICKSIM_VERSION_MAJOR}_${BRICKSIM_VERSION_MINOR}_${BRICKSIM_VERSION_PATCH}")

set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/resources\\\\logo_icon.png")

#set(CPACK_INSTALL_CMAKE_PROJECTS BrickSim)

if(WIN32)
    SET(CPACK_GENERATOR "WIX")

    set(CPACK_RESOURCE_FILE_LICENSE  "${CMAKE_SOURCE_DIR}/LICENSE.txt")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/resources/logo_icon.ico")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER BrickSim)

    # There is a bug in NSI that does not handle full UNIX paths properly.
    # Make sure there is at least one set of four backlashes.
    #SET(CPACK_NSIS_MUI_ICON "${CMake_SOURCE_DIR}/resources\\\\logo_icon.ico")
    #SET(CPACK_NSIS_MUI_UNIICON "${CMake_SOURCE_DIR}/resources\\\\logo_icon_uninstall.ico")

    #set(CPACK_NSIS_MENU_LINKS "programs\\bin\\BrickSim" "BrickSim")

    ###set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
    #set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} BrickSim")
    #set(CPACK_NSIS_HELP_LINK "https:\\\\\\\\bb1950328.github.io/BrickSim/")
    #set(CPACK_NSIS_URL_INFO_ABOUT "https:\\\\\\\\bb1950328.github.io/BrickSim/")
    ##set(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
endif()

if(UNIX)
    SET(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_CONTACT bb1950328@gmail.com)
    set(CPACK_PACKAGE_FILE_NAME BrickSim)

    #For some reason, shlibdeps doesn't work on the GitHub Actions server. that's why the dependencies are hardcoded
    #uncomment the next line, run cpack locally and run 'dpkg --info _CPack_Packages/Linux/DEB/BrickSim.deb' if you want to update the dependencies
    #set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.29), libcurl4 (>= 7.16.2), libgcc-s1 (>= 3.0), libgl1, libglew2.1 (>= 2.1.0), libstdc++6 (>= 9), libx11-6, libzip5 (>= 0.10), libopengl0")
endif()

include(CPack)