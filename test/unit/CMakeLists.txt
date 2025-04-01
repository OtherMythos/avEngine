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
