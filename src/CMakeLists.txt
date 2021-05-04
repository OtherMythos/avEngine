include_directories(.)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

file(GLOB_RECURSE srcs "*.cpp")
file(GLOB_RECURSE hdrs "*.h")

list(REMOVE_ITEM srcs "${CMAKE_PROJECT_DIR}/src/Platforms/Main.cpp")

set(TEST_MODE TRUE CACHE BOOL "Enable or disable the engine testing capabilities.")
set(DEBUGGING_TOOLS TRUE CACHE BOOL "Enable or disable the engine developer tools.")

if(TEST_MODE)
    add_compile_definitions(TEST_MODE)
endif()

if(DEBUGGING_TOOLS)
    add_compile_definitions(DEBUGGING_TOOLS)
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
    Platforms/Main.cpp

    #TODO get rid of this.
    ${AV_LIBS_DIR}/tinyxml2/tinyxml2.cpp
)

add_executable(${CMAKE_PROJECT_NAME}
    Platforms/Main.cpp
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
