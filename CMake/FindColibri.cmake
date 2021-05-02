
FIND_PATH(Colibri_INCLUDE_DIR ColibriWidget.h
    PATHS ${Colibri_ROOT}
        ENV Colibri_ROOT
    PATH_SUFFIXES include/ColibriGui)

FIND_LIBRARY(Colibri_LIBRARY NAMES ColibriGui
    PATHS ${Colibri_ROOT}/build/${CMAKE_BUILD_TYPE}
    PATH_SUFFIXES a lib
    )

SET(Colibri_FOUND FALSE)
IF(Colibri_INCLUDE_DIR AND Colibri_LIBRARY)
   SET(Colibri_FOUND TRUE)
ENDIF(Colibri_INCLUDE_DIR AND Colibri_LIBRARY)

IF(Colibri_FOUND)
   IF (NOT Colibri_FIND_QUIETLY)
	   MESSAGE(STATUS "Found ColibriGUI: ${Colibri_LIBRARY}")
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
