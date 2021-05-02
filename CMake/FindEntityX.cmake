
FIND_PATH(EntityX_INCLUDE_DIR entityx.h
    PATHS ${EntityX_ROOT}
        ENV EntityX_ROOT
    PATH_SUFFIXES entityx)

FIND_LIBRARY(EntityX_LIBRARY NAMES entityx entityx-d
    PATHS ${EntityX_ROOT}/build/${CMAKE_BUILD_TYPE}
    PATH_SUFFIXES a lib
    )

SET(EntityX_FOUND FALSE)
IF(EntityX_INCLUDE_DIR AND EntityX_LIBRARY)
   SET(EntityX_FOUND TRUE)
ENDIF(EntityX_INCLUDE_DIR AND EntityX_LIBRARY)

IF(EntityX_FOUND)
   IF (NOT EntityX_FIND_QUIETLY)
	   MESSAGE(STATUS "Found EntityX: ${EntityX_LIBRARY}")
   ENDIF (NOT EntityX_FIND_QUIETLY)
ELSE(EntityX_FOUND)
   IF (EntityX_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find EntityX")
   ENDIF (EntityX_FIND_REQUIRED)
ENDIF(EntityX_FOUND)

mark_as_advanced( FORCE
    EntityX_INCLUDE_DIR
    EntityX_LIBRARY
)
