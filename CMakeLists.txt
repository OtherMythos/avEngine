project(av)
cmake_minimum_required(VERSION 2.8)

set(AV_LIBS_DIR "/home/edward/Documents/avDeps" CACHE FILEPATH "")

set(OGRE_TARGET_BUILD_TYPE "Release")
set(SQUIRREL_TARGET_BUILD_TYPE "Debug")
set(SDL2_TARGET_BUILD_TYPE "Debug")

if(APPLE)
    set(AV_LIBS_DIR "/Users/edward/Documents/avDeps")
endif()

if(WIN32)
    set(AV_LIBS_DIR "C:/Users/edward/Documents/avDeps")
endif()

if(TEST_MODE)
    add_definitions(-DTEST_MODE)
endif()

if(DEBUGGING_TOOLS)
    add_definitions(-DDEBUGGING_TOOLS)
endif()

file(GLOB_RECURSE srcs "src/*.cpp")
file(GLOB_RECURSE hdrs "src/*.h")

file(GLOB imguiCpp
    "${AV_LIBS_DIR}/imgui/*.cpp"
)

list(REMOVE_ITEM srcs "${CMAKE_PROJECT_DIR}/src/Platforms/Main.cpp")

include_directories(src)

include_directories(${AV_LIBS_DIR}/spdlog/include)

include_directories(${AV_LIBS_DIR}/filesystem/)

include_directories(${AV_LIBS_DIR}/ogre/OgreMain/include)
include_directories(${AV_LIBS_DIR}/ogre/Components/Hlms/Pbs/include)
include_directories(${AV_LIBS_DIR}/ogre/Components/Hlms/Unlit/include)
include_directories(${AV_LIBS_DIR}/ogre/Components/Hlms/Common/include)

#include_directories(${AV_LIBS_DIR}/rapidjson/include)

include_directories(${AV_LIBS_DIR}/imgui/)

include_directories(${AV_LIBS_DIR}/tinyxml2/)

include_directories(${AV_LIBS_DIR}/squirrel/include)

include_directories(${AV_LIBS_DIR}/entityx)

include_directories(${AV_LIBS_DIR}/bullet3/src)

if(WIN32)
	set(CMAKE_CXX_STANDARD 11)
	message("Building for Windows")
    set(CMAKE_BUILD_TYPE "Debug")

    set(OGRE_TARGET_BUILD_TYPE "Debug")

    include_directories("${AV_LIBS_DIR}/ogre/Dependencies/src/rapidjsonInclude/include")

    include_directories(${AV_LIBS_DIR}/SDL2/include)
    include_directories(${AV_LIBS_DIR}/SDL2/build/include)

	include_directories(${AV_LIBS_DIR}/ogre/build/include)
    include_directories(${AV_LIBS_DIR}/ogre/OgreMain/include/WIN32)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/GL3Plus/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/GL3Plus/include/GLSL)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/Direct3D11/include)

	add_executable(${CMAKE_PROJECT_NAME}
        WIN32
        src/Platforms/Main.cpp
		${srcs}
        ${hdrs}

		${imguiCpp}
    )

    if(MSVC AND ${MSVC_VERSION} GREATER_EQUAL 1915)
        # You must acknowledge that you understand MSVC resolved a byte alignment issue in this compiler
        # We get this due to using Eigen objects and allocating those objects with make_shared
        target_compile_definitions( ${CMAKE_PROJECT_NAME} PRIVATE _DISABLE_EXTENDED_ALIGNED_STORAGE )
    endif()

    message(${AV_LIBS_DIR}/ogre/build/lib/${OGRE_TARGET_BUILD_TYPE})
    find_library(OGRE OgreMain_d.lib HINTS ${AV_LIBS_DIR}/ogre/build/lib/${OGRE_TARGET_BUILD_TYPE})
    find_library(RENDER_SYSTEM_OPENGL RenderSystem_GL3Plus_d.lib HINTS ${AV_LIBS_DIR}/ogre/build/lib/${OGRE_TARGET_BUILD_TYPE})
    find_library(RENDER_SYSTEM_D3D11 RenderSystem_Direct3D11_d.lib HINTS ${AV_LIBS_DIR}/ogre/build/lib/${OGRE_TARGET_BUILD_TYPE})
    find_library(OGRE_HLMS_PBS OgreHlmsPbs_d.lib HINTS ${AV_LIBS_DIR}/ogre/build/lib/${OGRE_TARGET_BUILD_TYPE})
    find_library(OGRE_HLMS_UNLIT OgreHlmsUnlit_d.lib HINTS ${AV_LIBS_DIR}/ogre/build/lib/${OGRE_TARGET_BUILD_TYPE})
	find_library(ENTITYX_LIB entityx-d.lib HINTS ${AV_LIBS_DIR}/entityx/build/Debug)

    #To build bullet I had to change the lib slightly in the visual studio settings.
    #lib in the vs menu > properties > c/c++ > Code Generation > Runtime Library. This needed to be the same as all the other libs (including the engine) generated for the projects.
    find_library(BULLET_COLLISION BulletCollision_Debug.lib HINTS ${AV_LIBS_DIR}/bullet3/build/lib/Debug)
    find_library(BULLET_DYNAMICS BulletDynamics_Debug.lib HINTS ${AV_LIBS_DIR}/bullet3/build/lib/Debug)
    find_library(BULLET_LINEARMATH LinearMath_Debug.lib HINTS ${AV_LIBS_DIR}/bullet3/build/lib/Debug)

    message("Copying files to ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    file(COPY
        ${AV_LIBS_DIR}/ogre/build/bin/${OGRE_TARGET_BUILD_TYPE}/OgreMain_d.dll
        ${AV_LIBS_DIR}/ogre/build/bin/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_GL3Plus_d.dll
        ${AV_LIBS_DIR}/ogre/build/bin/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_Direct3D11_d.dll
        ${AV_LIBS_DIR}/ogre/build/bin/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsPbs_d.dll
        ${AV_LIBS_DIR}/ogre/build/bin/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsUnlit_d.dll
        ${AV_LIBS_DIR}/entityx/build/Debug/entityx-d.dll

        ${AV_LIBS_DIR}/ogre/Samples/Media/Hlms
        ${CMAKE_CURRENT_LIST_DIR}/setup/windows/avSetup.cfg
        ${CMAKE_CURRENT_LIST_DIR}/setup/essential
    DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")


    target_link_libraries(${CMAKE_PROJECT_NAME}
        winmm.lib
        version.lib
        oleaut32.lib
        imm32.lib
		${AV_LIBS_DIR}/SDL2/build/${SDL2_TARGET_BUILD_TYPE}/SDL2.lib

        ${OGRE}
        ${RENDER_SYSTEM_OPENGL}
        ${RENDER_SYSTEM_D3D11}
        ${OGRE_HLMS_PBS}
        ${OGRE_HLMS_UNLIT}

		${ENTITYX_LIB}

        ${AV_LIBS_DIR}/squirrel/build/squirrel/${SQUIRREL_TARGET_BUILD_TYPE}/squirrel_static.lib
        ${AV_LIBS_DIR}/squirrel/build/sqstdlib/${SQUIRREL_TARGET_BUILD_TYPE}/sqstdlib_static.lib

        ${BULLET_COLLISION}
        ${BULLET_DYNAMICS}
        ${BULLET_LINEARMATH}

    )

endif()

if(UNIX AND NOT APPLE)
    message("Building for Linux")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

    set(CMAKE_BUILD_TYPE "Debug")

    include_directories(${AV_LIBS_DIR}/ogre/build/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/GL3Plus/include)
    include_directories(${AV_LIBS_DIR}/ogre/RenderSystems/GL3Plus/include/GLSL)

    include_directories("/usr/include/SDL2")
    include_directories("${AV_LIBS_DIR}/ogre/Dependencies/include/")

    add_library(avCore SHARED
        ${srcs}
        ${hdrs}

        ${imguiCpp}
        ${AV_LIBS_DIR}/tinyxml2/tinyxml2.cpp
    )

    add_executable(${CMAKE_PROJECT_NAME}
        src/Platforms/Main.cpp
    )


    find_library(OGRE OgreMain HINTS ${AV_LIBS_DIR}/ogre/build/lib/)
    find_library(RENDER_SYSTEM_OPENGL RenderSystem_GL3Plus.so HINTS ${AV_LIBS_DIR}/ogre/build/lib/)
    find_library(OGRE_HLMS_PBS OgreHlmsPbs HINTS ${AV_LIBS_DIR}/ogre/build/lib/)
    find_library(OGRE_HLMS_UNLIT OgreHlmsUnlit HINTS ${AV_LIBS_DIR}/ogre/build/lib/)

    find_library(BULLET3COLLISION BulletCollision HINTS ${AV_LIBS_DIR}/bullet3/build/src/BulletCollision)
    find_library(BULLET3DYNAMICS BulletDynamics HINTS ${AV_LIBS_DIR}/bullet3/build/src/BulletDynamics)
    find_library(BULLETLINEARMATH LinearMath HINTS ${AV_LIBS_DIR}/bullet3/build/src/LinearMath)


    find_library(ENTITYX entityx HINTS ${AV_LIBS_DIR}/entityx/build)

    file(COPY ${AV_LIBS_DIR}/ogre/Samples/Media/Hlms DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/essential DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/avSetup.cfg DESTINATION ${CMAKE_BINARY_DIR})

    message("Symlink to be ${RENDER_SYSTEM_OPENGL} -> ${CMAKE_BINARY_DIR}/RenderSystem_GL3Plus.so")
    add_custom_command(
      TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
      COMMAND ln -sf ${RENDER_SYSTEM_OPENGL} ${CMAKE_BINARY_DIR}/RenderSystem_GL3Plus.so
    )

    target_link_libraries(avCore
        SDL2
        dl

        ${OGRE}
        ${RENDER_SYSTEM_OPENGL}
        ${OGRE_HLMS_PBS}
        ${OGRE_HLMS_UNLIT}

        ${BULLET3COLLISION}
        ${BULLET3DYNAMICS}
        ${BULLETLINEARMATH}

        ${ENTITYX}

    )

    target_link_libraries(${CMAKE_PROJECT_NAME}
        avCore

        ${AV_LIBS_DIR}/squirrel/build/squirrel/libsquirrel_static.a
        ${AV_LIBS_DIR}/squirrel/build/sqstdlib/libsqstdlib_static.a
    )

endif()

if(APPLE)
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

    file(GLOB OGRE_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/Ogre.framework)
    file(GLOB OGRE_METAL_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_Metal.framework)
    file(GLOB OGRE_GL3PLUS_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_GL3Plus.framework)
    file(GLOB OGRE_HLMS_PBS_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsPbs.framework)
    file(GLOB OGRE_HLMS_UNLIT_FRAMEWORK ${AV_LIBS_DIR}/ogre/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsUnlit.framework)

    include_directories("${AV_LIBS_DIR}/ogre/Dependencies/include/")

    set_source_files_properties(
        ${OGRE_FRAMEWORK}
        ${OGRE_METAL_FRAMEWORK}
        ${OGRE_HLMS_PBS_FRAMEWORK}
        ${OGRE_GL3PLUS_FRAMEWORK}
        ${OGRE_HLMS_UNLIT_FRAMEWORK}
    PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)


    file(GLOB OGRE_HLMS_SHADERS
        ${AV_LIBS_DIR}/ogre/Samples/Media/Hlms
        ${CMAKE_CURRENT_LIST_DIR}/setup/mac/avSetup.cfg
        ${CMAKE_CURRENT_LIST_DIR}/setup/essential
    )
    set_source_files_properties(${OGRE_HLMS_SHADERS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    add_executable(${CMAKE_PROJECT_NAME}
        MACOSX_BUNDLE
        src/Platforms/Main.cpp
        ${srcs}
        ${hdrs}
        src/Window/SDL2Window/MacOS/MacOSUtils.mm
        ${imguiCpp}
        ${AV_LIBS_DIR}/tinyxml2/tinyxml2.cpp

        ${OGRE_FRAMEWORK}
        ${OGRE_METAL_FRAMEWORK}
        ${OGRE_GL3PLUS_FRAMEWORK}
        ${OGRE_HLMS_PBS_FRAMEWORK}
        ${OGRE_HLMS_UNLIT_FRAMEWORK}

        ${OGRE_HLMS_SHADERS}
    )

    set_source_files_properties(src/Gui/Developer/imguiOgre/imguiManager.cpp
src/Gui/Developer/imguiOgre/imguiRenderable.cpp
         PROPERTIES COMPILE_FLAGS "-x objective-c++")

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

        ${AV_LIBS_DIR}/bullet3/build/src/BulletCollision/Debug/libBulletCollision.a
        ${AV_LIBS_DIR}/bullet3/build/src/BulletDynamics/Debug/libBulletDynamics.a
        ${AV_LIBS_DIR}/bullet3/build/src/LinearMath/Debug/libLinearMath.a

        ${AV_LIBS_DIR}/entityx/build/Debug/libentityx-d.dylib
    )

endif()

add_subdirectory(test/unit)
