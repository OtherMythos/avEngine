

if(NOT EXISTS ${AV_LIBS_DIR})
    message(FATAL_ERROR "Provided dependencies path does not exist.\n${AV_LIBS_DIR}")
endif()

#TODO autoset CMAKE_BUILD_TYPE

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

#Removes an error relating to the _ROOT variables being set
if(${CMAKE_VERSION} VERSION_GREATER "3.12.0")
    cmake_policy(SET CMP0074 NEW)
endif()

set(Squirrel_ROOT ${AV_LIBS_DIR}/squirrel CACHE PATH "Root directory where Squirrel is installed")
set(Colibri_ROOT ${AV_LIBS_DIR}/colibri CACHE PATH "Root directory where ColibriGUI is installed")
set(EntityX_ROOT ${AV_LIBS_DIR}/entityx CACHE PATH "Root directory where EntityX is installed")
set(Ogre_ROOT ${AV_LIBS_DIR}/ogre2 CACHE PATH "Root directory where Ogre is installed")
set(Bullet_ROOT ${AV_LIBS_DIR}/bullet3 CACHE PATH "Root directory where Bullet is installed")
set(Detour_ROOT ${AV_LIBS_DIR}/recastdetour CACHE PATH "Root directory where recastdetour is installed")
set(SDL2_ROOT ${AV_LIBS_DIR}/SDL2 CACHE PATH "Root directory where SDL2 is installed")
set(OpenALSoft_ROOT ${AV_LIBS_DIR}/OpenALSoft CACHE PATH "Root directory where OpenALSoft is installed")
set(LibSndFile_ROOT ${AV_LIBS_DIR}/libsndfile CACHE PATH "Root directory where libsndfile is installed")
set(RLottie_ROOT ${AV_LIBS_DIR}/rlottie CACHE PATH "Root directory where RLottie is installed")

find_package(Squirrel REQUIRED)
find_package(Colibri REQUIRED)
find_package(EntityX REQUIRED)
find_package(Ogre REQUIRED)
find_package(Bullet REQUIRED)
find_package(Detour REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenALSoft REQUIRED)
find_package(LibSndFile REQUIRED)
find_package(RLottie REQUIRED)

FIND_PATH(RAPIDJSON_INCLUDE rapidjson
    PATHS ${AV_LIBS_DIR}/rapidjson/include/
    NO_DEFAULT_PATH
    REQUIRED)
message(${RAPIDJSON_INCLUDE})
