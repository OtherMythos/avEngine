
FIND_PATH(Colibri_INCLUDE_DIR ColibriGui
    PATHS ${Colibri_ROOT}
        ENV Colibri_ROOT
    PATH_SUFFIXES include/)

IF(Colibri_INCLUDE_DIR)
    set(Colibri_INCLUDE_DIR "${Colibri_INCLUDE_DIR};${Colibri_ROOT}/Dependencies/MSVC_Fix/2012" CACHE STRING "" FORCE)
    if(MSVC90)
        set(Colibri_INCLUDE_DIR "${Colibri_INCLUDE_DIR};${Colibri_ROOT}/Dependencies/MSVC_Fix" CACHE STRING "" FORCE)
    endif()
ENDIF()

#All the libraries should be static only.
FIND_LIBRARY(Colibri_LIBRARY NAMES ColibriGui
    PATHS ${Colibri_ROOT}/build/${CMAKE_BUILD_TYPE}
    PATH_SUFFIXES a
    )

FIND_LIBRARY(COLIBRI_FREETYPE NAMES freetype
    PATHS ${Colibri_ROOT}/build/${CMAKE_BUILD_TYPE}/Dependencies/freetype2
    PATH_SUFFIXES a
    )

FIND_LIBRARY(COLIBRI_HARFBUZZ NAMES harfbuzz
    PATHS ${Colibri_ROOT}/build/${CMAKE_BUILD_TYPE}/Dependencies/harfbuzz
    PATH_SUFFIXES a
    )

FIND_LIBRARY(COLIBRI_HARFBUZZ_SUBSET NAMES harfbuzz-subset
    PATHS ${Colibri_ROOT}/build/${CMAKE_BUILD_TYPE}/Dependencies/harfbuzz
    PATH_SUFFIXES a
    )

FIND_LIBRARY(COLIBRI_ICU NAMES icucommon
    PATHS ${Colibri_ROOT}/build/${CMAKE_BUILD_TYPE}/Dependencies/icu
    PATH_SUFFIXES a
    )

FIND_LIBRARY(COLIBRI_ZLIB NAMES z
    PATHS ${Colibri_ROOT}/build/${CMAKE_BUILD_TYPE}/Dependencies/zlib
    PATH_SUFFIXES a
    )

if(Colibri_LIBRARY)
    set(Colibri_LIBRARY "${Colibri_LIBRARY};${COLIBRI_FREETYPE};${COLIBRI_HARFBUZZ};${COLIBRI_HARFBUZZ_SUBSET};${COLIBRI_ICU};${COLIBRI_ZLIB}" CACHE STRING "" FORCE)
endif()

SET(Colibri_FOUND FALSE)
IF(Colibri_INCLUDE_DIR AND Colibri_LIBRARY)
   SET(Colibri_FOUND TRUE)
ENDIF(Colibri_INCLUDE_DIR AND Colibri_LIBRARY)

IF(Colibri_FOUND)
   IF (NOT Colibri_FIND_QUIETLY)
	   MESSAGE(STATUS "Found ColibriGUI: ${Colibri_LIBRARY}")
       MESSAGE(STATUS "Found ColibriGUI includes: ${Colibri_INCLUDE_DIR}")
   ENDIF (NOT Colibri_FIND_QUIETLY)
ELSE(Colibri_FOUND)
   IF (Colibri_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find ColibriGUI")
   ENDIF (Colibri_FIND_REQUIRED)
ENDIF(Colibri_FOUND)

mark_as_advanced( FORCE
    Colibri_INCLUDE_DIR
    Colibri_LIBRARY
)
