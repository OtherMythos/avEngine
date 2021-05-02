

FIND_PATH(Squirrel_INCLUDE_DIR squirrel.h
    PATHS ${Squirrel_ROOT}
        ENV Squirrel_ROOT
    PATH_SUFFIXES include)

FIND_LIBRARY(Squirrel_LIBRARY NAMES squirrel
    PATHS ${Squirrel_ROOT}/build/${CMAKE_BUILD_TYPE}
        ENV Squirrel_ROOT
    PATH_SUFFIXES lib)

FIND_LIBRARY(Squirrel_STD_LIBRARY NAMES sqstdlib
    PATHS ${Squirrel_ROOT}/build/${CMAKE_BUILD_TYPE}
        ENV Squirrel_ROOT
    PATH_SUFFIXES lib)

SET(Squirrel_FOUND FALSE)
IF(Squirrel_INCLUDE_DIR AND Squirrel_STD_LIBRARY)
   SET(Squirrel_FOUND TRUE)
ENDIF(Squirrel_INCLUDE_DIR AND Squirrel_STD_LIBRARY)

IF(Squirrel_FOUND)
   IF (NOT Squirrel_FIND_QUIETLY)
	   MESSAGE(STATUS "Found Squirrel: ${Squirrel_LIBRARY}, ${Squirrel_STD_LIBRARY}")
   ENDIF (NOT Squirrel_FIND_QUIETLY)
ELSE(Squirrel_FOUND)
   IF (Squirrel_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Squirrel")
   ENDIF (Squirrel_FIND_REQUIRED)
ENDIF(Squirrel_FOUND)

mark_as_advanced( FORCE
    Squirrel_INCLUDE_DIR
    Squirrel_STD_LIBRARY
    Squirrel_LIBRARY
)
