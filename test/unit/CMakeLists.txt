project(avUnit)
cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE srcs_test "src/*.cpp")
file(GLOB_RECURSE hdrs_test "src/*.h")

include_directories(${AV_LIBS_DIR}/googletest/googletest/include)
include_directories(${AV_LIBS_DIR}/googletest/googlemock/include)

if(UNIX AND NOT APPLE)
    add_executable(avUnit
        ${srcs_test}
        ${hdrss_test}
    )

    target_link_libraries(avUnit
        ${AV_LIBS_DIR}/googletest/build/lib/libgtest.a
        ${AV_LIBS_DIR}/googletest/build/lib/libgmock.a

        avCore

        ${AV_LIBS_DIR}/squirrel/build/squirrel/libsquirrel_static.a
        ${AV_LIBS_DIR}/squirrel/build/sqstdlib/libsqstdlib_static.a
    )
endif()

if(APPLE)
    list(REMOVE_ITEM srcs "${CMAKE_SOURCE_DIR}/src/Platforms/Main.cpp")

    set(APP_BUNDLE_IDENTIFIER "com.edward.av")

    set_source_files_properties(
        ${OGRE_FRAMEWORK}
        ${OGRE_METAL_FRAMEWORK}
        ${OGRE_HLMS_PBS_FRAMEWORK}
        ${OGRE_GL3PLUS_FRAMEWORK}
        ${OGRE_HLMS_UNLIT_FRAMEWORK}
    PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)

    add_executable(avUnit
        MACOSX_BUNDLE
        ${srcs}
        ${hdrs}
        ${srcs_test}
        ${hdrss_test}
        ${CMAKE_SOURCE_DIR}/src/Window/SDL2Window/MacOS/MacOSUtils.mm

        ${OGRE_FRAMEWORK}
        ${OGRE_METAL_FRAMEWORK}
        ${OGRE_GL3PLUS_FRAMEWORK}
        ${OGRE_HLMS_PBS_FRAMEWORK}
        ${OGRE_HLMS_UNLIT_FRAMEWORK}
    )

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks")

    set_target_properties(
        avUnit
        PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ${CMAKE_SOURCE_DIR}/setup/plist.in
    )

    target_link_libraries(avUnit
        ${AV_LIBS_DIR}/googletest/build/lib/libgtest.a
        ${AV_LIBS_DIR}/googletest/build/lib/libgmock.a
        ${AV_LIBS_DIR}/SDL2/build/libSDL2.a
        ${Frameworks}
        ${iconv}

        ${OGRE}
        ${RENDER_SYSTEM_METAL}
        ${RENDER_SYSTEM_OPENGL}
        ${OGRE_HLMS_UNLIT}
        ${OGRE_HLMS_PBS}

        ${AV_LIBS_DIR}/squirrel/build/squirrel/${SQUIRREL_TARGET_BUILD_TYPE}/libsquirrel_static.a
        ${AV_LIBS_DIR}/squirrel/build/sqstdlib/${SQUIRREL_TARGET_BUILD_TYPE}/libsqstdlib_static.a

        ${AV_LIBS_DIR}/entityx/build/Debug/libentityx-d.dylib
    )

endif()
