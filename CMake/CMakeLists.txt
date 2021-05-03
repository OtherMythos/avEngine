

if(NOT EXISTS ${AV_LIBS_DIR})
    message(FATAL_ERROR "Provided dependencies path does not exist.\n${AV_LIBS_DIR}")
endif()

#Removes an error relating to the _ROOT variables being set
cmake_policy(SET CMP0074 NEW)
set(Squirrel_ROOT ${AV_LIBS_DIR}/squirrel CACHE PATH "Root directory where Squirrel is installed")
set(Colibri_ROOT ${AV_LIBS_DIR}/colibri CACHE PATH "Root directory where ColibriGUI is installed")
set(EntityX_ROOT ${AV_LIBS_DIR}/entityx CACHE PATH "Root directory where EntityX is installed")
set(Ogre_ROOT ${AV_LIBS_DIR}/ogre2 CACHE PATH "Root directory where Ogre is installed")
set(Bullet_ROOT ${AV_LIBS_DIR}/bullet3 CACHE PATH "Root directory where Bullet is installed")
set(Detour_ROOT ${AV_LIBS_DIR}/recastdetour CACHE PATH "Root directory where recastdetour is installed")

find_package(Squirrel REQUIRED)
find_package(Colibri REQUIRED)
find_package(EntityX REQUIRED)
find_package(Ogre REQUIRED)
find_package(Bullet REQUIRED)
find_package(Detour REQUIRED)

FIND_PATH(RAPIDJSON_INCLUDE rapidjson
    PATHS ${AV_LIBS_DIR}/rapidjson/include/)
