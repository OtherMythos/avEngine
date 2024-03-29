#Had to write my own findSDL2 as it was playing up on windows.

message("finding sdl2")
FIND_PATH(SDL2_INCLUDE_DIRS SDL.h
    PATHS ${SDL2_ROOT}/include
        ENV SDL2_ROOT
    PATH_SUFFIXES include/ include/SDL2)

set(SDL2_LIBRARIES "" CACHE STRING "" FORCE)

FIND_LIBRARY(SDL2_MAIN_LIB NAMES SDL2main SDL2maind
    PATHS ${SDL2_ROOT}/lib/x64 ${SDL2_ROOT}/lib/
    PATH_SUFFIXES a lib
    NO_DEFAULT_PATH
    )

FIND_LIBRARY(SDL2_LIB NAMES SDL2 SDL2d
    PATHS ${SDL2_ROOT}/lib/x64 ${SDL2_ROOT}/lib/
    PATH_SUFFIXES a lib
    NO_DEFAULT_PATH
    )

IF(WIN32)
    FIND_FILE(SDL2_DLL SDL2.dll SDL2d.dll
        PATHS ${SDL2_ROOT}/bin
        PATH_SUFFIXES dll
        )
endif()

if(APPLE)
if("${PLATFORM}" STREQUAL "OS64")
    FIND_FILE(SDL2_MAIN_FILE SDL_uikit_main.c
        PATHS ${SDL2_ROOT}/src/main/uikit)
endif()
endif()

if(SDL2_MAIN_LIB)
  set(SDL2_LIBRARIES "${SDL2_MAIN_LIB};${SDL2_LIB}" CACHE STRING "" FORCE)
endif()
message("${SDL2_ROOT}/lib/x64")
message("FOUND: ${SDL2_LIB}")
message("FOUND: ${SDL2_LIBRARIES}")

SET(SDL2_FOUND FALSE)
IF(SDL2_INCLUDE_DIRS AND SDL2_LIBRARIES)
   SET(SDL2_FOUND TRUE)
ENDIF(SDL2_INCLUDE_DIRS AND SDL2_LIBRARIES)

IF(SDL2_FOUND)
   IF (NOT SDL2_FIND_QUIETLY)
       MESSAGE(STATUS "Found SDL2: ${SDL2_LIBRARIES}")
       MESSAGE(STATUS "Found SDL2 includes: ${SDL2_INCLUDE_DIRS}")
   ENDIF (NOT SDL2_FIND_QUIETLY)
ELSE(SDL2_FOUND)
   IF (SDL2_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find SDL2")
   ENDIF (SDL2_FIND_REQUIRED)
ENDIF(SDL2_FOUND)

mark_as_advanced( FORCE
    SDL2_INCLUDE_DIRS
)
