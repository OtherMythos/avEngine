project(av)
cmake_minimum_required(VERSION 3.1)

#Default to debug
set(CMAKE_BUILD_TYPE "Debug")

set(AV_LIBS_DIR "${CMAKE_SOURCE_DIR}/Dependencies" CACHE FILEPATH "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
add_subdirectory(CMake)

add_subdirectory(src)

file(COPY ${Ogre_ROOT}/Samples/Media/Hlms DESTINATION ${CMAKE_BINARY_DIR}/src)
file(COPY ${AV_LIBS_DIR}/colibri/bin/Data/Hlms/Colibri DESTINATION ${CMAKE_BINARY_DIR}/src/Hlms/)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/essential DESTINATION ${CMAKE_BINARY_DIR}/src)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/avSetup.cfg DESTINATION ${CMAKE_BINARY_DIR}/src)

message("Symlink to be ${LIB_RENDERSYSTEM_OPENGL} -> ${CMAKE_BINARY_DIR}/src/RenderSystem_GL3Plus.so")
message("ln -sf ${LIB_RENDERSYSTEM_OPENGL} ${CMAKE_BINARY_DIR}/src/RenderSystem_GL3Plus.so")
# add_custom_command(
#     TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
#     COMMAND ln -sf ${LIB_RENDERSYSTEM_OPENGL} ${CMAKE_BINARY_DIR}/src/RenderSystem_GL3Plus.so
# )

# set(OGRE_TARGET_BUILD_TYPE "Release")
# set(SQUIRREL_TARGET_BUILD_TYPE "Debug")
# set(SDL2_TARGET_BUILD_TYPE "Debug")

# set(TEST_MODE TRUE CACHE BOOL "Enable or disable the engine testing capabilities.")
# set(DEBUGGING_TOOLS TRUE CACHE BOOL "Enable or disable the engine developer tools.")

# if(APPLE)
#     set(AV_LIBS_DIR "/Users/edward/Documents/avDeps")
# endif()

# if(WIN32)
#     set(AV_LIBS_DIR "C:/Users/edward/Documents/avDeps")
# endif()

# set(AV_LIBS_OGRE_DIR "${AV_LIBS_DIR}/ogre2")

# if(TEST_MODE)
#     add_definitions(-DTEST_MODE)
# endif()

# if(DEBUGGING_TOOLS)
#     add_definitions(-DDEBUGGING_TOOLS)
# endif()

# add_definitions(-DOGRE_IGNORE_UNKNOWN_DEBUG)

# file(GLOB_RECURSE srcs "src/*.cpp")
# file(GLOB_RECURSE hdrs "src/*.h")

# list(REMOVE_ITEM srcs "${CMAKE_PROJECT_DIR}/src/Platforms/Main.cpp")

# include_directories(src)

# include_directories(${AV_LIBS_DIR}/spdlog/include)

# include_directories(${AV_LIBS_DIR}/filesystem/)

# include_directories(${AV_LIBS_OGRE_DIR}/OgreMain/include)
# include_directories(${AV_LIBS_OGRE_DIR}/Components/Hlms/Pbs/include)
# include_directories(${AV_LIBS_OGRE_DIR}/Components/Hlms/Unlit/include)
# include_directories(${AV_LIBS_OGRE_DIR}/Components/Hlms/Common/include)

# #include_directories(${AV_LIBS_DIR}/rapidjson/include)

# include_directories(${AV_LIBS_DIR}/tinyxml2/)

# include_directories(${AV_LIBS_DIR}/squirrel/include)

# include_directories(${AV_LIBS_DIR}/entityx)

# include_directories(${AV_LIBS_DIR}/bullet3/src)

# include_directories("${AV_LIBS_DIR}/colibrigui/include")
# include_directories( "${AV_LIBS_DIR}/colibrigui/Dependencies/MSVC_Fix/2012" )
# if( MSVC90 )
#     include_directories( "${AV_LIBS_DIR}/colibrigui/Dependencies/MSVC_Fix" )
# endif()
# include_directories( "${AV_LIBS_DIR}/colibrigui/Dependencies/harfbuzz/src" )

# include_directories(${AV_LIBS_DIR}/recastnavigation/Recast/Include)
# include_directories(${AV_LIBS_DIR}/recastnavigation/Detour/Include)
# if(DEBUGGING_TOOLS)
#     include_directories(${AV_LIBS_DIR}/recastnavigation/DebugUtils/Include)
# endif()
# include_directories(${AV_LIBS_DIR}/recastnavigation/DetourTileCache/Include)

# if(WIN32)
# 	set(CMAKE_CXX_STANDARD 11)
# 	message("Building for Windows")
#     set(CMAKE_BUILD_TYPE "Debug")

#     set(OGRE_TARGET_BUILD_TYPE "Debug")

#     include_directories("${AV_LIBS_OGRE_DIR}/Dependencies/src/rapidjsonInclude/include")

#     include_directories(${AV_LIBS_DIR}/SDL2/include)
#     include_directories(${AV_LIBS_DIR}/SDL2/build/include)

# 	include_directories(${AV_LIBS_OGRE_DIR}/build/include)
#     include_directories(${AV_LIBS_OGRE_DIR}/OgreMain/include/WIN32)
#     include_directories(${AV_LIBS_OGRE_DIR}/RenderSystems/GL3Plus/include)
#     include_directories(${AV_LIBS_OGRE_DIR}/RenderSystems/GL3Plus/include/GLSL)
#     include_directories(${AV_LIBS_OGRE_DIR}/RenderSystems/Direct3D11/include)

# 	add_executable(${CMAKE_PROJECT_NAME}
#         #WIN32
#         src/Platforms/Main.cpp
# 		${srcs}
#         ${hdrs}

#         ${AV_LIBS_DIR}/tinyxml2/tinyxml2.cpp
#     )

#     if(MSVC AND ${MSVC_VERSION} GREATER_EQUAL 1915)
#         # You must acknowledge that you understand MSVC resolved a byte alignment issue in this compiler
#         # We get this due to using Eigen objects and allocating those objects with make_shared
#         target_compile_definitions( ${CMAKE_PROJECT_NAME} PRIVATE _DISABLE_EXTENDED_ALIGNED_STORAGE )
#     endif()

#     message(${AV_LIBS_OGRE_DIR}/build/lib/${OGRE_TARGET_BUILD_TYPE})
#     find_library(OGRE OgreMain_d.lib HINTS ${AV_LIBS_OGRE_DIR}/build/lib/${OGRE_TARGET_BUILD_TYPE})
#     find_library(RENDER_SYSTEM_OPENGL RenderSystem_GL3Plus_d.lib HINTS ${AV_LIBS_OGRE_DIR}/build/lib/${OGRE_TARGET_BUILD_TYPE})
#     find_library(RENDER_SYSTEM_D3D11 RenderSystem_Direct3D11_d.lib HINTS ${AV_LIBS_OGRE_DIR}/build/lib/${OGRE_TARGET_BUILD_TYPE})
#     find_library(OGRE_HLMS_PBS OgreHlmsPbs_d.lib HINTS ${AV_LIBS_OGRE_DIR}/build/lib/${OGRE_TARGET_BUILD_TYPE})
#     find_library(OGRE_HLMS_UNLIT OgreHlmsUnlit_d.lib HINTS ${AV_LIBS_OGRE_DIR}/build/lib/${OGRE_TARGET_BUILD_TYPE})
# 	find_library(ENTITYX_LIB entityx-d.lib HINTS ${AV_LIBS_DIR}/entityx/build/Debug)

#     #To build bullet I had to change the lib slightly in the visual studio settings.
#     #lib in the vs menu > properties > c/c++ > Code Generation > Runtime Library. This needed to be the same as all the other libs (including the engine) generated for the projects.
#     find_library(BULLET_COLLISION BulletCollision_Debug.lib HINTS ${AV_LIBS_DIR}/bullet3/build/lib/Debug)
#     find_library(BULLET_DYNAMICS BulletDynamics_Debug.lib HINTS ${AV_LIBS_DIR}/bullet3/build/lib/Debug)
#     find_library(BULLET_LINEARMATH LinearMath_Debug.lib HINTS ${AV_LIBS_DIR}/bullet3/build/lib/Debug)

#     message("Copying files to ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
#     file(COPY
#         ${AV_LIBS_OGRE_DIR}/build/bin/${OGRE_TARGET_BUILD_TYPE}/OgreMain_d.dll
#         ${AV_LIBS_OGRE_DIR}/build/bin/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_GL3Plus_d.dll
#         ${AV_LIBS_OGRE_DIR}/build/bin/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_Direct3D11_d.dll
#         ${AV_LIBS_OGRE_DIR}/build/bin/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsPbs_d.dll
#         ${AV_LIBS_OGRE_DIR}/build/bin/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsUnlit_d.dll
#         ${AV_LIBS_DIR}/entityx/build/Debug/entityx-d.dll

#         ${AV_LIBS_OGRE_DIR}/Samples/Media/Hlms
#         ${CMAKE_CURRENT_LIST_DIR}/setup/windows/avSetup.cfg
#         ${CMAKE_CURRENT_LIST_DIR}/setup/essential
#     DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

#     file(COPY ${AV_LIBS_DIR}/colibrigui/bin/Data/Hlms/Colibri DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Hlms/)

#     if(DEBUGGING_TOOLS)
#         set(RECAST_DEBUG_LIBS ${AV_LIBS_DIR}/recastnavigation/build/DebugUtils/Debug/DebugUtils.lib)
#     endif()


#     target_link_libraries(${CMAKE_PROJECT_NAME}
#         winmm.lib
#         version.lib
#         oleaut32.lib
#         imm32.lib
#         ${AV_LIBS_DIR}/SDL2/build/${SDL2_TARGET_BUILD_TYPE}/SDL2.lib

#         ${OGRE}
#         ${RENDER_SYSTEM_OPENGL}
#         ${RENDER_SYSTEM_D3D11}
#         ${OGRE_HLMS_PBS}
#         ${OGRE_HLMS_UNLIT}

#         ${ENTITYX_LIB}

#         ${AV_LIBS_DIR}/squirrel/build/squirrel/${SQUIRREL_TARGET_BUILD_TYPE}/squirrel_static.lib
#         ${AV_LIBS_DIR}/squirrel/build/sqstdlib/${SQUIRREL_TARGET_BUILD_TYPE}/sqstdlib_static.lib

#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/${CMAKE_BUILD_TYPE}/ColibriGui.lib
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/${CMAKE_BUILD_TYPE}/freetype.lib
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/${CMAKE_BUILD_TYPE}/harfbuzz.lib
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/${CMAKE_BUILD_TYPE}/harfbuzz-subset.lib
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/${CMAKE_BUILD_TYPE}/icucommon.lib
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/${CMAKE_BUILD_TYPE}/zlibstatic.lib

#         ${AV_LIBS_DIR}/recastnavigation/build/Recast/Debug/Recast.lib
#         ${AV_LIBS_DIR}/recastnavigation/build/Detour/Debug/Detour.lib
#         ${RECAST_DEBUG_LIBS}
#         ${AV_LIBS_DIR}/recastnavigation/build/DetourTileCache/Debug/DetourTileCache.lib

#         ${BULLET_COLLISION}
#         ${BULLET_DYNAMICS}
#         ${BULLET_LINEARMATH}

#     )

# endif()

# if(UNIX AND NOT APPLE)
#     message("Building for Linux")

#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

#     set(CMAKE_BUILD_TYPE "Debug")

#     include_directories(${AV_LIBS_OGRE_DIR}/build/include)
#     include_directories(${AV_LIBS_OGRE_DIR}/RenderSystems/GL3Plus/include)
#     include_directories(${AV_LIBS_OGRE_DIR}/RenderSystems/GL3Plus/include/GLSL)

#     include_directories("/usr/include/SDL2")
#     include_directories("${AV_LIBS_OGRE_DIR}/Dependencies/include/")

#     add_library(avCore SHARED
#         ${srcs}
#         ${hdrs}

#         ${AV_LIBS_DIR}/tinyxml2/tinyxml2.cpp
#     )

#     add_executable(${CMAKE_PROJECT_NAME}
#         src/Platforms/Main.cpp
#     )


#     find_library(OGRE OgreMain HINTS ${AV_LIBS_OGRE_DIR}/build/lib/)
#     find_library(RENDER_SYSTEM_OPENGL RenderSystem_GL3Plus.so HINTS ${AV_LIBS_OGRE_DIR}/build/lib/)
#     find_library(OGRE_HLMS_PBS OgreHlmsPbs HINTS ${AV_LIBS_OGRE_DIR}/build/lib/)
#     find_library(OGRE_HLMS_UNLIT OgreHlmsUnlit HINTS ${AV_LIBS_OGRE_DIR}/build/lib/)

#     find_library(BULLET3COLLISION BulletCollision HINTS ${AV_LIBS_DIR}/bullet3/build/src/BulletCollision)
#     find_library(BULLET3DYNAMICS BulletDynamics HINTS ${AV_LIBS_DIR}/bullet3/build/src/BulletDynamics)
#     find_library(BULLETLINEARMATH LinearMath HINTS ${AV_LIBS_DIR}/bullet3/build/src/LinearMath)


#     find_library(ENTITYX entityx HINTS ${AV_LIBS_DIR}/entityx/build)

#     file(COPY ${AV_LIBS_OGRE_DIR}/Samples/Media/Hlms DESTINATION ${CMAKE_BINARY_DIR})
#     file(COPY ${AV_LIBS_DIR}/colibrigui/bin/Data/Hlms/Colibri DESTINATION ${CMAKE_BINARY_DIR}/Hlms/)
#     file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/essential DESTINATION ${CMAKE_BINARY_DIR})
#     file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/avSetup.cfg DESTINATION ${CMAKE_BINARY_DIR})

#     message("Symlink to be ${RENDER_SYSTEM_OPENGL} -> ${CMAKE_BINARY_DIR}/RenderSystem_GL3Plus.so")
#     add_custom_command(
#       TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
#       COMMAND ln -sf ${RENDER_SYSTEM_OPENGL} ${CMAKE_BINARY_DIR}/RenderSystem_GL3Plus.so
#     )

#     if(DEBUGGING_TOOLS)
#         set(RECAST_DEBUG_LIBS ${AV_LIBS_DIR}/recastnavigation/build/DebugUtils/libDebugUtils.a)
#     endif()

#     target_link_libraries(avCore
#         SDL2
#         dl

#         ${OGRE}
#         ${RENDER_SYSTEM_OPENGL}
#         ${OGRE_HLMS_PBS}
#         ${OGRE_HLMS_UNLIT}

#         ${BULLET3COLLISION}
#         ${BULLET3DYNAMICS}
#         ${BULLETLINEARMATH}

#         ${AV_LIBS_DIR}/recastnavigation/build/Recast/libRecast.a
#         ${AV_LIBS_DIR}/recastnavigation/build/Detour/libDetour.a
#         ${RECAST_DEBUG_LIBS}
#         ${AV_LIBS_DIR}/recastnavigation/build/DetourTileCache/libDetourTileCache.a

#         ${ENTITYX}

#     )

#     target_link_libraries(${CMAKE_PROJECT_NAME}
#         avCore

#         ${AV_LIBS_DIR}/squirrel/build/squirrel/libsquirrel_static.a
#         ${AV_LIBS_DIR}/squirrel/build/sqstdlib/libsqstdlib_static.a

#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/RelWithDebInfo/libColibriGui.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/RelWithDebInfo/libfreetype.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/RelWithDebInfo/libharfbuzz.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/RelWithDebInfo/libharfbuzz-subset.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/RelWithDebInfo/libicucommon.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/RelWithDebInfo/libz.a
#     )

# endif()

# if(APPLE)
#     message("Building for MacOS")

#     set(OGRE_TARGET_BUILD_TYPE "Debug")

#     set(CMAKE_CXX_FLAGS "-x objective-c++")
#     set(CMAKE_SIZEOF_VOID_P 4)
#     set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
#     set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
#     set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

#     set(APP_BUNDLE_IDENTIFIER "com.edward.av")

#     set(MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER})
#     set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
#     set(MACOSX_BUNDLE_BUNDLE_NAME "av")

#     file(GLOB OGRE_FRAMEWORK ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/Ogre.framework)
#     file(GLOB OGRE_METAL_FRAMEWORK ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_Metal.framework)
#     #file(GLOB OGRE_GL3PLUS_FRAMEWORK ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/RenderSystem_GL3Plus.framework)
#     file(GLOB OGRE_HLMS_PBS_FRAMEWORK ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsPbs.framework)
#     file(GLOB OGRE_HLMS_UNLIT_FRAMEWORK ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE}/OgreHlmsUnlit.framework)

#     include_directories("${AV_LIBS_OGRE_DIR}/Dependencies/include/")

#     set_source_files_properties(
#         ${OGRE_FRAMEWORK}
#         ${OGRE_METAL_FRAMEWORK}
#         ${OGRE_HLMS_PBS_FRAMEWORK}
#         #${OGRE_GL3PLUS_FRAMEWORK}
#         ${OGRE_HLMS_UNLIT_FRAMEWORK}
#     PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)


#     file(GLOB OGRE_HLMS_SHADERS
#         ${AV_LIBS_OGRE_DIR}/Samples/Media/Hlms
#         ${CMAKE_CURRENT_LIST_DIR}/setup/mac/avSetup.cfg
#         ${CMAKE_CURRENT_LIST_DIR}/setup/essential
#     )
#     set_source_files_properties(${OGRE_HLMS_SHADERS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

#     add_executable(${CMAKE_PROJECT_NAME}
#         MACOSX_BUNDLE
#         src/Platforms/Main.cpp
#         ${srcs}
#         ${hdrs}
#         src/Window/SDL2Window/MacOS/MacOSUtils.mm
#         ${AV_LIBS_DIR}/tinyxml2/tinyxml2.cpp

#         ${OGRE_FRAMEWORK}
#         ${OGRE_METAL_FRAMEWORK}
#         #${OGRE_GL3PLUS_FRAMEWORK}
#         ${OGRE_HLMS_PBS_FRAMEWORK}
#         ${OGRE_HLMS_UNLIT_FRAMEWORK}

#         ${OGRE_HLMS_SHADERS}
#     )

#     include_directories(${AV_LIBS_DIR}/SDL2/include)
#     include_directories(${AV_LIBS_DIR}/SDL2/build/include)

#     include_directories(${AV_LIBS_OGRE_DIR}/build/include)
#     include_directories(${AV_LIBS_OGRE_DIR}/RenderSystems/Metal/include)
#     include_directories(${AV_LIBS_OGRE_DIR}/RenderSystems/GL3Plus/include)
#     include_directories(${AV_LIBS_OGRE_DIR}/RenderSystems/GL3Plus/include/GLSL)

#     set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks")

#     set_target_properties(
#         ${CMAKE_PROJECT_NAME}
#         PROPERTIES
#         MACOSX_BUNDLE_INFO_PLIST
#         ${CMAKE_CURRENT_LIST_DIR}/setup/plist.in
#     )

#     set(Frameworks
#         "-framework AudioToolbox"
#         "-framework CoreAudio"
#         "-framework CoreVideo"
#         "-framework CoreFoundation"
#         "-framework Carbon"
#         "-framework Cocoa"
#         "-framework IOKit"
#         "-framework ForceFeedback"
#         "-framework QuartzCore"
#         "-framework Metal"
#         "-framework OpenGL"
#     )

#     find_library(iconv libiconv.tbd)

#     find_library(OGRE Ogre HINTS ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})
#     find_library(RENDER_SYSTEM_METAL RenderSystem_Metal HINTS ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})
#     #find_library(RENDER_SYSTEM_OPENGL RenderSystem_GL3Plus HINTS ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})
#     find_library(OGRE_HLMS_PBS OgreHlmsPbs HINTS ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})
#     find_library(OGRE_HLMS_UNLIT OgreHlmsUnlit HINTS ${AV_LIBS_OGRE_DIR}/build/lib/macosx/${OGRE_TARGET_BUILD_TYPE})

#     target_link_libraries(${CMAKE_PROJECT_NAME}
#         ${AV_LIBS_DIR}/SDL2/build/libSDL2.a
#         ${Frameworks}
#         ${iconv}

#         ${OGRE}
#         ${RENDER_SYSTEM_METAL}
#         #${RENDER_SYSTEM_OPENGL}
#         ${OGRE_HLMS_UNLIT}
#         ${OGRE_HLMS_PBS}

#         ${AV_LIBS_DIR}/squirrel/build/squirrel/${SQUIRREL_TARGET_BUILD_TYPE}/libsquirrel_static.a
#         ${AV_LIBS_DIR}/squirrel/build/sqstdlib/${SQUIRREL_TARGET_BUILD_TYPE}/libsqstdlib_static.a

#         ${AV_LIBS_DIR}/bullet3/build/src/BulletCollision/Debug/libBulletCollision.a
#         ${AV_LIBS_DIR}/bullet3/build/src/BulletDynamics/Debug/libBulletDynamics.a
#         ${AV_LIBS_DIR}/bullet3/build/src/LinearMath/Debug/libLinearMath.a

#         ${AV_LIBS_DIR}/entityx/build/Debug/libentityx-d.dylib

#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/Debug/Debug/libColibriGui.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/Debug/Debug/libfreetype.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/Debug/Debug/libharfbuzz.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/Debug/Debug/libharfbuzz-subset.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/Debug/Debug/libicucommon.a
#         ${AV_LIBS_DIR}/colibrigui/Dependencies/build_result/lib/Debug/Debug/libz.a
#     )

# endif()

# add_subdirectory(test/unit)
