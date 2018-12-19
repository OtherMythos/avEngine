project(av)
cmake_minimum_required(VERSION 2.8)

set(AV_LIBS_DIR "/Users/edward/Documents/avDeps" CACHE FILEPATH "")

file(GLOB_RECURSE srcs "src/*.cpp")
file(GLOB_RECURSE hdrs "src/*.h")

list(REMOVE_ITEM srcs "${CMAKE_PROJECT_DIR}/src/Platforms/Main.cpp")

include_directories(${AV_LIBS_DIR}/spdlog/include)

include_directories(${AV_LIBS_DIR}/ogre/OgreMain/include)
include_directories(${AV_LIBS_DIR}/ogre/Components/Hlms/Pbs/include)
include_directories(${AV_LIBS_DIR}/ogre/Components/Hlms/Common/include)

if(UNIX AND NOT APPLE)
    message("Building for Linux")

    include_directories("/usr/include/SDL2")

    add_executable(${CMAKE_PROJECT_NAME} src/Main.cpp ${srcs} ${hdrs})

    target_link_libraries(${CMAKE_PROJECT_NAME} SDL2)

endif()

if(APPLE)
	message("Building for MacOS")
    set(CMAKE_CXX_FLAGS "-x objective-c++ -stdlib=libc++ -std=c++11")

    set(APP_BUNDLE_IDENTIFIER "com.edward.av")

    set(MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME "av")

    file(GLOB OGRE_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/Debug/Ogre.framework)
    file(GLOB OGRE_METAL_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/Debug/RenderSystem_Metal.framework)
    file(GLOB OGRE_HLMS_PBS_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/Debug/OgreHlmsPbs.framework)
    set_source_files_properties(${OGRE_FRAMEWORK} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
    set_source_files_properties(${OGRE_METAL_FRAMEWORK} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
    set_source_files_properties(${OGRE_HLMS_PBS_FRAMEWORK} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)

    file(GLOB OGRE_HLMS_PBS_SHADERS ${AV_LIBS_DIR}/ogre/Samples/Media/Hlms)
    set_source_files_properties(${OGRE_HLMS_PBS_SHADERS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    add_executable(${CMAKE_PROJECT_NAME}
        MACOSX_BUNDLE
        src/Platforms/Main.cpp
        ${srcs}
        ${hdrs}

        ${OGRE_FRAMEWORK}
        ${OGRE_METAL_FRAMEWORK}
        ${OGRE_HLMS_PBS_FRAMEWORK}

        ${OGRE_HLMS_PBS_SHADERS}
    )

    include_directories(${AV_LIBS_DIR}/SDL2/include)
    include_directories(${AV_LIBS_DIR}/SDL2/build/include)

    include_directories(${AV_LIBS_DIR}/ogre/build/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/Metal/include)


    include_directories(src)

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
    )

    find_library(iconv libiconv.tbd)

    find_library(OGRE Ogre HINTS ${AV_LIBS_DIR}/ogre/build/lib/macosx/Debug)
    find_library(RENDER_SYSTEM_METAL RenderSystem_Metal HINTS ${AV_LIBS_DIR}/ogre/build/lib/macosx/Debug)
    find_library(OGRE_HLMS_PBS OgreHlmsPbs HINTS ${AV_LIBS_DIR}/ogre/build/lib/macosx/Debug)

    target_link_libraries(${CMAKE_PROJECT_NAME}
        ${AV_LIBS_DIR}/SDL2/build/libSDL2.a
        ${Frameworks}
        ${iconv}
        ${OGRE}
        ${RENDER_SYSTEM_METAL}
        ${OGRE_HLMS_PBS}
    )

endif()

add_subdirectory(test/unit)
