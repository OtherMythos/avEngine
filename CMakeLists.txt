project(av)
cmake_minimum_required(VERSION 2.8)

set(AV_LIBS_DIR "/home/edward/Documents/avDeps" CACHE FILEPATH "")

file(GLOB_RECURSE srcs "src/*.cpp")
file(GLOB_RECURSE hdrs "src/*.h")

list(REMOVE_ITEM srcs "${CMAKE_PROJECT_DIR}/src/Platforms/Main.cpp")

include_directories(src)

include_directories(${AV_LIBS_DIR}/spdlog/include)

include_directories(${AV_LIBS_DIR}/ogre/OgreMain/include)
include_directories(${AV_LIBS_DIR}/ogre/Components/Hlms/Pbs/include)
include_directories(${AV_LIBS_DIR}/ogre/Components/Hlms/Unlit/include)
include_directories(${AV_LIBS_DIR}/ogre/Components/Hlms/Common/include)

include_directories(${AV_LIBS_DIR}/squirrel/include)

if(UNIX AND NOT APPLE)
    message("Building for Linux")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

    #include_directories(${AV_LIBS_DIR}/SDL2/include)
    #include_directories(${AV_LIBS_DIR}/SDL2/build/include)
    #include_directories("/usr/include/SDL2")

    include_directories(${AV_LIBS_DIR}/ogre/build/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/GL3Plus/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/GL3Plus/include/GLSL)

    add_executable(${CMAKE_PROJECT_NAME}
        src/Platforms/Main.cpp
        ${srcs}
        ${hdrs}
    )

    include_directories("/usr/include/SDL2")

    find_library(OGRE OgreMain HINTS ${AV_LIBS_DIR}/ogre/build/lib/)
    find_library(RENDER_SYSTEM_OPENGL RenderSystem_GL3Plus.so HINTS ${AV_LIBS_DIR}/ogre/build/lib/)
    find_library(OGRE_HLMS_PBS OgreHlmsPbs HINTS ${AV_LIBS_DIR}/ogre/build/lib/)
    find_library(OGRE_HLMS_UNLIT OgreHlmsUnlit HINTS ${AV_LIBS_DIR}/ogre/build/lib/)

    file(COPY ${AV_LIBS_DIR}/ogre/Samples/Media/Hlms DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/avSetup.cfg DESTINATION ${CMAKE_BINARY_DIR})

    target_link_libraries(${CMAKE_PROJECT_NAME}
        #${AV_LIBS_DIR}/SDL2/build/libSDL2.a
        SDL2
        dl

        ${OGRE}
        ${RENDER_SYSTEM_OPENGL}
        ${OGRE_HLMS_PBS}
        ${OGRE_HLMS_UNLIT}

        ${AV_LIBS_DIR}/squirrel/build/squirrel/libsquirrel_static.a
        ${AV_LIBS_DIR}/squirrel/build/sqstdlib/libsqstdlib_static.a

    )

endif()

if(APPLE)
    set(AV_LIBS_DIR "/Users/edward/Documents/avDeps" CACHE FILEPATH "")
    message("Building for MacOS")

    set(CMAKE_CXX_FLAGS "-x objective-c++")
    set(CMAKE_SIZEOF_VOID_P 4)
    set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

    set(APP_BUNDLE_IDENTIFIER "com.edward.av")

    set(MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME "av")

    set(OGRE_TARGET_BUILD_TYPE "Release")
    set(SQUIRREL_TARGET_BUILD_TYPE "Debug")

    file(GLOB OGRE_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/Ogre.framework)
    file(GLOB OGRE_METAL_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_Metal.framework)
    file(GLOB OGRE_GL3PLUS_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_GL3Plus.framework)
    file(GLOB OGRE_HLMS_PBS_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsPbs.framework)
    file(GLOB OGRE_HLMS_UNLIT_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsUnlit.framework)
    set_source_files_properties(${OGRE_FRAMEWORK} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
    set_source_files_properties(${OGRE_METAL_FRAMEWORK} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
    set_source_files_properties(${OGRE_GL3PLUS_FRAMEWORK} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
    set_source_files_properties(${OGRE_HLMS_PBS_FRAMEWORK} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
    set_source_files_properties(${OGRE_HLMS_UNLIT_FRAMEWORK} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)

    file(GLOB OGRE_HLMS_SHADERS ${AV_LIBS_DIR}/ogre/Samples/Media/Hlms ${CMAKE_CURRENT_LIST_DIR}/setup/mac/avSetup.cfg)
    set_source_files_properties(${OGRE_HLMS_SHADERS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    add_executable(${CMAKE_PROJECT_NAME}
        MACOSX_BUNDLE
        src/Platforms/Main.cpp
        ${srcs}
        ${hdrs}
        src/Window/SDL2Window/MacOS/MacOSUtils.mm

        ${OGRE_FRAMEWORK}
        ${OGRE_METAL_FRAMEWORK}
        ${OGRE_GL3PLUS_FRAMEWORK}
        ${OGRE_HLMS_PBS_FRAMEWORK}
        ${OGRE_HLMS_UNLIT_FRAMEWORK}

        ${OGRE_HLMS_SHADERS}
    )

    include_directories(${AV_LIBS_DIR}/SDL2/include)
    include_directories(${AV_LIBS_DIR}/SDL2/build/include)

    include_directories(${AV_LIBS_DIR}/ogre/build/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/Metal/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/GL3Plus/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/GL3Plus/include/GLSL)

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks")

    set_target_properties(
        ${CMAKE_PROJECT_NAME}
        PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ${CMAKE_CURRENT_LIST_DIR}/setup/plist.in
    )

    set(Frameworks
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreVideo"
        "-framework CoreFoundation"
        "-framework Carbon"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework ForceFeedback"
        "-framework QuartzCore"
        "-framework Metal"
        "-framework OpenGL"
    )

    find_library(iconv libiconv.tbd)

    find_library(OGRE Ogre HINTS ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})
    find_library(RENDER_SYSTEM_METAL RenderSystem_Metal HINTS ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})
    find_library(RENDER_SYSTEM_OPENGL RenderSystem_GL3Plus HINTS ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})
    find_library(OGRE_HLMS_PBS OgreHlmsPbs HINTS ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})
    find_library(OGRE_HLMS_UNLIT OgreHlmsUnlit HINTS ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})

    target_link_libraries(${CMAKE_PROJECT_NAME}
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
    )

endif()

#add_subdirectory(test/unit)
