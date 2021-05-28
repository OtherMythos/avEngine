project(av)
cmake_minimum_required(VERSION 3.1)

#Default to debug
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -D_DEBUG=1" )

set(AV_LIBS_DIR "${CMAKE_SOURCE_DIR}/Dependencies" CACHE FILEPATH "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
add_subdirectory(CMake)

include_directories(src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

file(GLOB_RECURSE srcs "src/*.cpp")
file(GLOB_RECURSE hdrs "src/*.h")

list(REMOVE_ITEM srcs "${CMAKE_PROJECT_DIR}/src/Platforms/Main.cpp")

set(TEST_MODE TRUE CACHE BOOL "Enable or disable the engine testing capabilities.")
set(DEBUGGING_TOOLS TRUE CACHE BOOL "Enable or disable the engine developer tools.")
set(BUILD_UNIT_TESTS TRUE CACHE BOOL "Build the unit tests.")

if(TEST_MODE)
    add_definitions(-DTEST_MODE)
endif()

if(DEBUGGING_TOOLS)
    add_definitions(-DDEBUGGING_TOOLS)
endif()

include_directories(${AV_LIBS_DIR}/spdlog/include)
include_directories(${AV_LIBS_DIR}/filesystem/)
include_directories(${AV_LIBS_DIR}/tinyxml2/)

find_package(SDL2 REQUIRED)

include_directories(
    ${Colibri_INCLUDE_DIR}
    ${Squirrel_INCLUDE_DIR}
    ${Ogre_INCLUDE_DIR}
    ${EntityX_INCLUDE_DIR}
    ${Bullet_INCLUDE_DIR}
    ${Detour_INCLUDE_DIR}

    ${RAPIDJSON_INCLUDE}

    ${SDL2_INCLUDE_DIRS}
)

add_library(avCore SHARED
    ${srcs}
    ${hdrs}
    src/Platforms/Main.cpp

    #TODO get rid of this.
    ${AV_LIBS_DIR}/tinyxml2/tinyxml2.cpp
)

add_executable(${CMAKE_PROJECT_NAME}
    src/Platforms/Main.cpp
)

target_link_libraries(avCore
    SDL2
    dl

    ${Squirrel_STD_LIBRARY}
    ${Squirrel_LIBRARY}
    ${Ogre_LIBRARY}
    ${EntityX_LIBRARY}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    avCore

    ${Colibri_LIBRARY}
    ${SDL2_LIBRARIES}
    ${Detour_LIBRARY}
    ${Bullet_LIBRARY}
)

if(${BUILD_UNIT_TESTS})
    message("Building unit tests")
    add_subdirectory(test/unit)
endif()

message("Symlink to be ${LIB_RENDERSYSTEM_OPENGL} -> ${CMAKE_BINARY_DIR}/RenderSystem_GL3Plus.so")
ADD_CUSTOM_TARGET(link_opengl ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIB_RENDERSYSTEM_OPENGL} ${CMAKE_BINARY_DIR}/RenderSystem_GL3Plus.so)

message("Symlink to be ${LIB_PARTICLE_FX} -> ${CMAKE_BINARY_DIR}/Plugin_ParticleFX.so")
ADD_CUSTOM_TARGET(link_particlefx ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIB_PARTICLE_FX} ${CMAKE_BINARY_DIR}/Plugin_ParticleFX.so)

file(COPY ${Ogre_ROOT}/Samples/Media/Hlms DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${AV_LIBS_DIR}/colibri/bin/Data/Hlms/Colibri DESTINATION ${CMAKE_BINARY_DIR}/Hlms/)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/essential DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/setup/avSetup.cfg DESTINATION ${CMAKE_BINARY_DIR})
