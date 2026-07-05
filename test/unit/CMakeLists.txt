project(avUnit)
cmake_minimum_required(VERSION 3.5)

#Removes an error relating to the _ROOT variables being set
if(${CMAKE_VERSION} VERSION_GREATER "3.12.0")
    cmake_policy(SET CMP0074 NEW)
endif()

set(GoogleTest_ROOT ${AV_LIBS_DIR}/googletest CACHE PATH "Root directory where googletest is installed")

find_package(GoogleTest REQUIRED)

file(GLOB_RECURSE srcs_test "src/*.cpp")
include_directories("${CMAKE_SOURCE_DIR}/test/")

include_directories(${GoogleTest_INCLUDE_DIR})

if(APPLE)
    #No avCore library exists on Apple platforms (the engine sources are compiled
    #directly into the av executable), so compile them into the test target as well.
    add_executable(avUnit
        ${srcs_test}
        ${srcs}
        ${hdrs}

        "${CMAKE_SOURCE_DIR}/src/Window/SDL2Window/MacOS/MacOSUtils.mm"
    )

    #The Ogre frameworks are copied into the av app bundle, so the test executable
    #needs to resolve them from their built location instead.
    foreach(_avLib ${Ogre_LIBRARY})
        if(_avLib)
            get_filename_component(AV_OGRE_LIB_DIR ${_avLib} DIRECTORY)
            break()
        endif()
    endforeach()
    set_target_properties(avUnit PROPERTIES
        BUILD_RPATH "${AV_OGRE_LIB_DIR}"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${AV_OGRE_LIB_DIR}"
    )
    #The Ogre frameworks have an @executable_path/../Frameworks install name,
    #so provide that path relative to the test executable.
    add_custom_command(TARGET avUnit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${AV_OGRE_LIB_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/Frameworks"
    )

    target_link_libraries(avUnit
        ${GoogleTest_LIBRARY}

        ${Squirrel_STD_LIBRARY}
        ${Squirrel_LIBRARY}
        ${Ogre_LIBRARY}
        ${EntityX_LIBRARY}
        ${SDL2_LIBRARIES}
        ${Colibri_LIBRARY}
        ${Detour_LIBRARY}
        ${Bullet_LIBRARY}
        ${OpenALSoft_LIBRARY}
        ${LibSndFile_LIBRARY}
        ${RLottie_LIBRARY}

        ${StaticPluginLibraries}

        ${Frameworks}
        ${iconv}

        git_version
    )
else()
    add_executable(avUnit
        ${srcs_test}
    )

    target_link_libraries(avUnit
        avCore

        ${GoogleTest_LIBRARY}

        ${Squirrel_STD_LIBRARY}
        ${Squirrel_LIBRARY}
        ${SDL2_LIBRARIES}
        ${Detour_LIBRARY}
        ${Bullet_LIBRARY}
        ${Colibri_LIBRARY}
        ${OpenALSoft_LIBRARY}
        ${LibSndFile_LIBRARY}
    )
endif()
