cmake_minimum_required(VERSION 3.5)
project(av)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SYSTEM_VERSION 24)

if("${PLATFORM}" STREQUAL "android")
    #TODOANDROID Remove the specific names
    set(NDK_LOCATION "" CACHE FILEPATH "")
    set(CMAKE_TOOLCHAIN_FILE "${NDK_LOCATION}/build/cmake/android.toolchain.cmake")

    set( APP_GLUE_DIR ${NDK_LOCATION}/sources/android/native_app_glue )
    include_directories( ${APP_GLUE_DIR} )
    add_library( app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c )

    set( ANDROID_CPU_FEATURES_DIR ${ANDROID_NDK}/sources/android/cpufeatures )
    include_directories( ${ANDROID_CPU_FEATURES_DIR} )
    add_library( cpufeatures STATIC ${ANDROID_CPU_FEATURES_DIR}/cpu-features.c )
endif()

include(CMake/CheckGit.cmake)
CheckGitSetup()

#Default to debug
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -D_DEBUG=1" )

set(AV_LIBS_DIR "${CMAKE_SOURCE_DIR}/Dependencies/avBuilt/${CMAKE_BUILD_TYPE}" CACHE FILEPATH "")
#TODO this should be more descriptive, i.e ios, android, etc . Not OS64.
set(PLATFORM "Desktop" CACHE STRING "")
set(AV_PROJECT_DIR "" CACHE FILEPATH "")

set(TEST_MODE TRUE CACHE BOOL "Enable or disable the engine testing capabilities.")
set(DEBUGGING_TOOLS TRUE CACHE BOOL "Enable or disable the engine developer tools.")
set(USE_STATIC_PLUGINS FALSE CACHE BOOL "Attempt to compile a 'StaticPlugins.h' file from the AV_PROJECT_DIR variable")
set(BUILD_UNIT_TESTS TRUE CACHE BOOL "Build the unit tests.")

if(${USE_STATIC_PLUGINS})
    if("${AV_PROJECT_DIR}" STREQUAL "")
        message( FATAL_ERROR "Requested USE_STATIC_PLUGINS without providing a AV_PROJECT_DIR value.")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
add_subdirectory(CMake)

include_directories(src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

file(GLOB_RECURSE srcs "src/*.cpp")
file(GLOB_RECURSE hdrs "src/*.h")

list(REMOVE_ITEM srcs "${PROJECT_SOURCE_DIR}/src/Platforms/Main.cpp")

set(ENTRY_FILE "src/Platforms/Main.cpp")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)

if(TEST_MODE)
    add_definitions(-DTEST_MODE)
endif()

if(DEBUGGING_TOOLS)
    add_definitions(-DDEBUGGING_TOOLS)
endif()

if(USE_STATIC_PLUGINS)
    add_definitions(-DUSE_STATIC_PLUGINS)

    add_subdirectory(${AV_PROJECT_DIR} projectStaticPlugins)
endif()

if("${PLATFORM}" STREQUAL "OS64")
    add_definitions(-DTARGET_APPLE_IPHONE)
endif()
if("${PLATFORM}" STREQUAL "android")
    add_definitions(-DTARGET_ANDROID)
endif()

include_directories(${AV_LIBS_DIR}/sds_library/include/sds)
include_directories(${AV_LIBS_DIR}/spdlog/include)
include_directories(${AV_LIBS_DIR}/filesystem/)
include_directories(${AV_LIBS_DIR}/tinyxml2/)

list(APPEND srcs
    "${AV_LIBS_DIR}/tinyxml2/tinyxml2.cpp"
)

add_compile_definitions(AL_LIBTYPE_STATIC)

include_directories(
    ${Colibri_INCLUDE_DIR}
    ${Squirrel_INCLUDE_DIR}
    ${Ogre_INCLUDE_DIR}
    ${EntityX_INCLUDE_DIR}
    ${Bullet_INCLUDE_DIR}
    ${Detour_INCLUDE_DIR}
    ${OpenALSoft_INCLUDE_DIR}
    ${LibSndFile_INCLUDE_DIR}
    ${RLottie_INCLUDE_DIR}

    ${RAPIDJSON_INCLUDE}

    ${SDL2_INCLUDE_DIRS}

    ${StaticPluginIncludes}
)

if(NOT APPLE AND NOT "${PLATFORM}" STREQUAL "android")
    add_library(avCore SHARED
        ${srcs}
        ${hdrs}
    )
endif()

if(APPLE)
    set(CMAKE_CXX_FLAGS "-x objective-c++")
    set(CMAKE_SIZEOF_VOID_P 4)
    set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

    set(APP_BUNDLE_IDENTIFIER "com.othermythos.av")
    set(PRODUCT_NAME "av")

    set(MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME "av")

    #TODO these are used by SDL2, but some of them seem unecessary i.e carbon.
    #Is there a way to get rid of them?
    set(Frameworks
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreVideo"
        "-framework CoreFoundation"
        "-framework QuartzCore"
        "-framework IOKit"
        "-framework Metal"
        "-framework CoreHaptics"
        "-framework GameController"
    )
    if("${PLATFORM}" STREQUAL "OS64")
        set(Frameworks
            ${Frameworks}
            "-framework CoreText"
            "-framework CoreGraphics"
            "-framework CoreHaptics"
            "-framework CoreBluetooth"
            "-framework CoreMotion"
            "-framework GameController"
            "-framework AVFAudio"
            "-framework UIKit"
            "-framework OpenGLES"
        )
    else()
        set(Frameworks
            ${Frameworks}
            "-framework Carbon"
            "-framework Cocoa"
            "-framework ForceFeedback"
        )
    endif()

    find_library(iconv libiconv.tbd)

    #Collect the files together in one place in the binary dir.
    #I have to do it like this here because CMake's facilities to copy to the macos bundle are pretty limited.
    #This makes it difficult to pull the Colibri Hlms files from a different location.
    execute_process (
        COMMAND bash -c "rm -rf ${CMAKE_BINARY_DIR}/Hlms;cp -r ${AV_LIBS_DIR}/ogre2/Media/Hlms ${CMAKE_BINARY_DIR}/Hlms;cp -r ${AV_LIBS_DIR}/colibri/data/Hlms/Colibri ${CMAKE_BINARY_DIR}/Hlms"
        OUTPUT_VARIABLE outVar
    )

    file(GLOB OGRE_HLMS_SHADERS
        ${CMAKE_CURRENT_LIST_DIR}/setup/essential
        ${CMAKE_BINARY_DIR}/Hlms
    )
    set_source_files_properties(${OGRE_HLMS_SHADERS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    set(executable_extras
        "src/Window/SDL2Window/MacOS/MacOSUtils.mm"

        ${OGRE_HLMS_SHADERS}
    )

    if("${PLATFORM}" STREQUAL "OS64")
        set(executable_extras
            ${executable_extras}
            ${CMAKE_CURRENT_LIST_DIR}/setup/ios/launchScreen.storyboard
            ${SDL2_MAIN_FILE}
        )
    else()
        #macos needs the libraries to include them as frameworks.
        set(executable_extras
            ${executable_extras}
            ${Ogre_LIBRARY}
        )
    endif()

    if(NOT "${AV_PROJECT_DIR}" STREQUAL "")
        #Remove any hidden files from the glob.
        if(NOT "${PROJ_TO_BUNDLE}" STREQUAL "")

            message("Copying to bundle: ${PROJ_TO_BUNDLE}")
            set_source_files_properties(${PROJ_TO_BUNDLE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

            set(executable_extras
                ${executable_extras}

                ${PROJ_TO_BUNDLE}
            )
        endif()
    endif()

endif()

if(APPLE)
    add_executable(${CMAKE_PROJECT_NAME}
        MACOSX_BUNDLE

        ${srcs}
        ${hdrs}
        ${ENTRY_FILE}

        ${executable_extras}
    )
    add_dependencies(git_version AlwaysCheckGit)
    add_dependencies(${PROJECT_NAME} git_version)
elseif("${PLATFORM}" STREQUAL "android")
    add_library(${CMAKE_PROJECT_NAME} SHARED
        ${srcs}
        ${hdrs}

        ${ENTRY_FILE}

        ${executable_extras}
    )
    add_dependencies(git_version AlwaysCheckGit)
else()
    add_executable(${CMAKE_PROJECT_NAME}
        ${ENTRY_FILE}
    )
endif()

if(APPLE)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks")

    if("${PLATFORM}" STREQUAL "OS64")
        set_target_properties(
            ${CMAKE_PROJECT_NAME}
            PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST
            ${CMAKE_CURRENT_LIST_DIR}/setup/ios/Info.plist
        )
    else()
        set_target_properties(
            ${CMAKE_PROJECT_NAME}
            PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST
            ${CMAKE_CURRENT_LIST_DIR}/setup/plist.in
        )
    endif()

    set_source_files_properties(
        ${Ogre_LIBRARY}
    PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
endif()

if(MSVC)
    if(${MSVC_VERSION} GREATER_EQUAL 1915)
        # You must acknowledge that you understand MSVC resolved a byte alignment issue in this compiler
        # We get this due to using Eigen objects and allocating those objects with make_shared
        target_compile_definitions( avCore PRIVATE _DISABLE_EXTENDED_ALIGNED_STORAGE )
    endif()
endif()

#TODO find a cleaner way than this to link for the two different platforms.
if("${PLATFORM}" STREQUAL "android")
    message("Building for Android")

    include_directories(/tmp/generated)

    target_link_libraries(${CMAKE_PROJECT_NAME}
        dl

        #TODO figure this out
        log
        vulkan
        android
        app-glue
        cpufeatures
        OpenSLES
        atomic
        ${iconv_lib}

        #${wayland_lib}

        ${Ogre_LIBRARY}
        ${EntityX_LIBRARY}

        ${Ogre_LIBRARY}
        ${EntityX_LIBRARY}
        ${RLottie_LIBRARY}
        ${Squirrel_STD_LIBRARY}
        ${Squirrel_LIBRARY}
        ${SDL2_LIBRARIES}
        ${Detour_LIBRARY}
        ${Bullet_LIBRARY}
        ${Colibri_LIBRARY}
        ${OpenALSoft_LIBRARY}
        ${LibSndFile_LIBRARY}

        ${StaticPluginLibraries}

        git_version

    )
elseif( UNIX AND NOT APPLE)
    find_library(wayland_lib
        NAMES wayland-client libwayland-client
    )

    include_directories(/tmp/generated)

    target_link_libraries(avCore
        dl

        ${wayland_lib}

        ${Ogre_LIBRARY}
        ${EntityX_LIBRARY}
        ${RLottie_LIBRARY}

        git_version
    )
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        message("Building for Linux")
    else()
        message("Building for BSD")
        include_directories(/usr/local/include/)
        find_library(iconv_lib
            NAMES iconv
            PATHS /usr/local/lib/
        )
        target_link_libraries(avCore
            ${iconv_lib}
            usbhid
        )
    endif()

    target_link_libraries(${CMAKE_PROJECT_NAME}
        avCore

        ${Squirrel_STD_LIBRARY}
        ${Squirrel_LIBRARY}
        ${SDL2_LIBRARIES}
        ${Detour_LIBRARY}
        ${Bullet_LIBRARY}
        ${Colibri_LIBRARY}
        ${OpenALSoft_LIBRARY}
        ${LibSndFile_LIBRARY}

        ${StaticPluginLibraries}
    )
elseif(APPLE)
    if("${PLATFORM}" STREQUAL "OS64")
        message("Building for iOS")

        set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
        )

        set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/setup/Assets.xcassets PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources
        )
        target_sources(av PRIVATE ${CMAKE_CURRENT_LIST_DIR}/setup/Assets.xcassets)

        set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME  "AppIcon"
        )

    else()
        message("Building for MacOS")
    endif()

    target_link_libraries(${CMAKE_PROJECT_NAME}
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

        ${StaticPluginLibraries}

        ${Frameworks}
        ${iconv}

        ${Colibri_LIBRARY}
        ${SDL2_LIBRARIES}
        ${Detour_LIBRARY}

        ${RLottie_LIBRARY}

        git_version
    )
else()
    message("Building for Windows")

    target_link_libraries(avCore
        winmm.lib
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

        git_version
    )

    target_link_libraries(${CMAKE_PROJECT_NAME}
        avCore

        ${Colibri_LIBRARY}
        ${SDL2_LIBRARIES}
        ${Detour_LIBRARY}
    )
endif()

if(${BUILD_UNIT_TESTS})
    message("Building unit tests")
    add_subdirectory(test/unit)
else()
    message("Not building unit tests")
endif()

if(UNIX AND NOT APPLE AND NOT "${PLATFORM}" STREQUAL "android")
    message("Symlink to be ${LIB_RENDERSYSTEM_OPENGL} -> ${CMAKE_BINARY_DIR}/RenderSystem_GL3Plus.so")
    ADD_CUSTOM_TARGET(link_opengl ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIB_RENDERSYSTEM_OPENGL} ${CMAKE_BINARY_DIR}/RenderSystem_GL3Plus.so)

    message("Symlink to be ${LIB_RENDERSYSTEM_VULKAN} -> ${CMAKE_BINARY_DIR}/RenderSystem_Vulkan.so")
    ADD_CUSTOM_TARGET(link_vulkan ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIB_RENDERSYSTEM_VULKAN} ${CMAKE_BINARY_DIR}/RenderSystem_Vulkan.so)

    message("Symlink to be ${LIB_PARTICLE_FX} -> ${CMAKE_BINARY_DIR}/Plugin_ParticleFX.so")
    ADD_CUSTOM_TARGET(link_particlefx ALL
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIB_PARTICLE_FX} ${CMAKE_BINARY_DIR}/Plugin_ParticleFX.so)
endif()

if(WIN32)
    #file(COPY ${Ogre_ROOT}/Media/Hlms DESTINATION ${CMAKE_BINARY_DIR})
    add_custom_command(TARGET av POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${AV_LIBS_DIR}/ogre2/Media/Hlms
        $<TARGET_FILE_DIR:av>/Hlms)
elseif(UNIX AND NOT APPLE)
    file(COPY ${Ogre_ROOT}/share/OGRE/Media/Hlms DESTINATION ${CMAKE_BINARY_DIR})
endif()

# add_custom_command(TARGET av PRE_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory
#         ${Ogre_ROOT}/Media/Hlms DESTINATION
#         $<TARGET_FILE_DIR:av>/Hlms)

#file(COPY ${AV_LIBS_DIR}/colibri/data/Hlms/Colibri DESTINATION ${CMAKE_BINARY_DIR}/Hlms/)
#file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/essential DESTINATION ${CMAKE_BINARY_DIR})
#file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/avSetup.cfg DESTINATION ${CMAKE_BINARY_DIR})

#Copy android resources to assets
if("${PLATFORM}" STREQUAL "android")
    set(COPY_COMMANDS "")
    foreach(item IN LISTS PROJ_TO_BUNDLE)
        get_filename_component(name "${item}" NAME)
        if(IS_DIRECTORY "${item}")
            list(APPEND COPY_COMMANDS
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${item}" "${CMAKE_CURRENT_LIST_DIR}/../assets/${name}")
        else()
            list(APPEND COPY_COMMANDS
                COMMAND ${CMAKE_COMMAND} -E copy "${item}" "${CMAKE_CURRENT_LIST_DIR}/../assets")
        endif()
    endforeach()

    if(NOT COPY_COMMANDS STREQUAL "")
        message("Copying resources: ${COPY_COMMANDS}")
	execute_process(
            ${COPY_COMMANDS}
        )
    else()
        message("No resources to copy into the asset bundle.")
    endif()
endif()

if(NOT APPLE)
    add_custom_command(TARGET av POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/setup/essential"
            $<TARGET_FILE_DIR:av>/essential)

    add_custom_command(TARGET av POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${AV_LIBS_DIR}/colibri/data/Hlms/Colibri
            $<TARGET_FILE_DIR:av>/Hlms/Colibri)

    if(WIN32)
        add_custom_command(TARGET av POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                ${OGRE_HLMS_PBS_DLL} ${OGRE_HLMS_UNLIT_DLL} ${OGRE_MAIN_DLL} ${OGRE_RENDER_GL_DLL} ${OGRE_RENDER_D3D11_DLL} ${OGRE_RENDER_VULKAN_DLL} ${OGRE_PLUGIN_PARTICLE_DLL} ${LIB_Z_DLL} ${SDL2_DLL} ${RLOTTIE_DLL} ${OGRE_AMD_ARGS}
                $<TARGET_FILE_DIR:av>)
    endif()
endif()
