

FIND_PATH(DETOUR_INCLUDE DetourMath.h
PATHS ${Detour_ROOT}
    ENV Detour_ROOT
PATH_SUFFIXES Detour/Include)

FIND_PATH(DETOUR_TILE_CACHE_INCLUDE DetourTileCache.h
PATHS ${Detour_ROOT}
    ENV Detour_ROOT
PATH_SUFFIXES DetourTileCache/Include)

FIND_PATH(DETOUR_DEBUG_UTILS DebugDraw.h
PATHS ${Detour_ROOT}
    ENV Detour_ROOT
PATH_SUFFIXES DebugUtils/Include)


FIND_LIBRARY(DETOUR_BASE_LIBRARY NAMES Detour Detour-d
PATHS ${Detour_ROOT}/build/${CMAKE_BUILD_TYPE}/Detour
    ENV Detour_ROOT
PATH_SUFFIXES lib a)

FIND_LIBRARY(DETOUR_TILE_CACHE_LIBRARY NAMES DetourTileCache DetourTileCache-d
PATHS ${Detour_ROOT}/build/${CMAKE_BUILD_TYPE}/DetourTileCache
    ENV Detour_ROOT
PATH_SUFFIXES lib a)

FIND_LIBRARY(DETOUR_DEBUG_UTILS_LIBRARY NAMES DebugUtils DebugUtils-d
PATHS ${Detour_ROOT}/build/${CMAKE_BUILD_TYPE}/DebugUtils
    ENV Detour_ROOT
PATH_SUFFIXES lib a)

message(${DETOUR_DEBUG_UTILS_LIBRARY})


set(Detour_LIBRARY "${Detour_LIBRARY};${DETOUR_BASE_LIBRARY};${DETOUR_TILE_CACHE_LIBRARY};${DETOUR_DEBUG_UTILS_LIBRARY}"
CACHE STRING "" FORCE)
set(Detour_INCLUDE_DIR "${Detour_INCLUDE_DIR};${DETOUR_INCLUDE};${DETOUR_TILE_CACHE_INCLUDE};${DETOUR_DEBUG_UTILS}"
CACHE STRING "" FORCE)

SET(Detour_FOUND FALSE)
IF(Detour_INCLUDE_DIR AND Detour_LIBRARY)
SET(Detour_FOUND TRUE)
ENDIF(Detour_INCLUDE_DIR AND Detour_LIBRARY)

IF(Detour_FOUND)
IF (NOT Detour_FIND_QUIETLY)
   MESSAGE(STATUS "Found Detour: ${Detour_LIBRARY}, ${Detour_INCLUDE_DIR}")
ENDIF (NOT Detour_FIND_QUIETLY)
ELSE(Detour_FOUND)
IF (Detour_FIND_REQUIRED)
  MESSAGE(FATAL_ERROR "Could not find Detour")
ENDIF (Detour_FIND_REQUIRED)
ENDIF(Detour_FOUND)

mark_as_advanced( FORCE
Detour_INCLUDE_DIR
Detour_LIBRARY
)