project(avUnit)
cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE srcs_test "src/*.cpp")
file(GLOB_RECURSE hdrs_test "src/*.h")

list(REMOVE_ITEM srcs "${CMAKE_SOURCE_DIR}/src/Platforms/Main.cpp")

include_directories(${AV_LIBS_DIR}/googletest/googletest/include)

add_executable(avUnit
    ${srcs}
    ${hdrs}

    ${srcs_test}
    ${hdrss_test}
)

target_link_libraries(avUnit
    ${AV_LIBS_DIR}/googletest/build/lib/libgtest.a
    ${AV_LIBS_DIR}/SDL2/build/libSDL2.a
    ${Frameworks}
    ${iconv}
)
